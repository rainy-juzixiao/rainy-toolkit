#include <iostream>
#include <rainy/meta/meta_method.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/reflection/moon/reflect.hpp>
#include <rainy/meta/reflection/registration.hpp>
#include <string_view>
#include <vector>

using namespace rainy;
using namespace rainy::collections::views;
using namespace rainy::foundation::rtti;
using namespace rainy::meta::reflection;

struct object {
    virtual std::string_view name() const noexcept;
};

struct point {
    point() = default;

    point(int x_, int y_) : x(x_), y(y_) {
    }

    void set(int x_, int y_) {
        this->x = x_;
        this->y = y_;
    }

    void addtional() {}

    int sum() const noexcept {
        return x + y;
    }

    int sum(int a, int b) const noexcept {
        return a + b;
    }

    int test_lref() && noexcept {
        std::cout << "这是右值引用\n";
        return 0;
    }

    int test_lref() & noexcept {
        std::cout << "这是左值引用\n";
        return 1;
    }

    int x{},y{};
};

template <>
struct rainy::meta::reflection::moon::reflect<object> : rainy::meta::reflection::moon::reflect_base<object> {
    static constexpr std::string_view name = rainy::foundation::rtti::type_name<object>();

    static constexpr auto methods = method_list{expose("name()", &object::name)};
};

template <>
struct rainy::meta::reflection::moon::reflect<point>
    : rainy::meta::reflection::moon::reflect_base<point, rainy::meta::reflection::moon::expose_base<object, true>> {
    static constexpr auto properties = property_list{expose("x", &point::x), expose_if<false>("y", &point::y)};

    static constexpr auto methods = method_list{expose("set", &point::set, bind_meta(attach("are_you_set", "yes"))),
                    expose_if<false>("sum", utility::get_overloaded_memfn<point, int() const noexcept>(&point::sum), bind_meta(attach("NBS", 1))),
                                                expose_if<true>("addtional", &point::addtional)};

    static constexpr auto ctors = method_list{expose("point()", rainy::meta::method::get_ctor_fn<point>()),
                                              expose("point(int, int)", rainy::meta::method::get_ctor_fn<point, int, int>())};
};

union s {
    s(int, int) {
        std::cout << "ctor\n";
    }
};
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <rttr/registration.h>

RTTR_REGISTRATION {
    rttr::registration::class_<point>("point")
        .method("sum", utility::get_overloaded_memfn<point, int() const noexcept>(&point::sum))(rttr::metadata("111", 1), rttr::metadata("222", 2))
        .method("l", [&]() { std::cout << "Hello Lambda\n"; })
        .method("set", &point::set)
        .property("x", &point::x)
        .constructor<int, int>()
        .constructor([](std::string_view a, std::string_view b) { return point{std::stoi(a.data()), std::stoi(b.data())}; });
}

RAINY_REFLECTION_REGISTRATION {
    rainy::meta::reflection::registration::class_<point>("point")
        .method("sum", utility::get_overloaded_memfn<point, int() const>(&point::sum), {{"111", 1}, {"222", 2}})
        .method("sum", utility::get_overloaded_memfn<point, int(int, int) const>(&point::sum), {{"111", 1}, {"222", 2}})
        .method("custom_static", [](int a, int b) { std::cout << "a+b == " << a + b << '\n'; })
        .method("fake", [](const point *p) { std::cout << "fake : p->x : " << p->x << "\n"; })
        .method("set", &point::set)
        .method("test_lref", utility::get_overloaded_memfn<point, int() &>(&point::test_lref))
        .method("test_lref", utility::get_overloaded_memfn<point, int() &&>(&point::test_lref))
        .property("x", &point::x)
        .constructor<int, int>()
        .constructor([](std::string_view a, std::string_view b) { return point{std::stoi(a.data()), std::stoi(b.data())}; });
}

void method___(float, float, int) {

}

int main() {
    rttr::type p_ttt = rttr::type::get<point>();
    auto m = p_ttt.get_method("l");
    if (m) {
        point p;
        m.invoke(p);
    }
    auto std_string_type = type::get<std::string>();
    for (const auto& item : std_string_type.get_methods()) {
        std::cout<< item.name() << " : " << item <<'\n';
    }
    std::string s = "111";

    std::cout << std_string_type.invoke_method(rainy::meta::method::cpp_methods::method_at, s, 1).type().name() << '\n';
    auto p_type = type::get_by_name("point");
    point pp;
    std::cout << p_type.invoke_method("sum", pp, 10, 20) << '\n';
    for (const auto& it : p_type.get_methods()) {
        std::cout << it.name() << " : " << it << '\n';
    }
    
    return 0;
}
