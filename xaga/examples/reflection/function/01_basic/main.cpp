#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/concurrency.hpp>
#include <rainy/foundation/io/timer.hpp>

#include <rainy/meta/moon/enumeration.hpp>
#include <rainy/meta/reflection/enumeration.hpp>

#if RAINY_USING_WINDOWS
#include <windows.h>
#endif
#include <iostream>
using namespace rainy;

namespace aaa {
    enum class enum_type {
        a = 4,
        b = 2,
        c = 3,
        d = 99999
    };
}

#if RAINY_HAS_CXX26_STATIC_REFLECTION && RAINY_HAS_CXX26
#include <print>

// clang-format off

struct[[ = rainy::annotations::moon::with_prefix("ty_"), = rainy::annotations::moon::with_suffix("_member") ]] type {
    [[= rainy::annotations::moon::ignore]]
    int a;
    [[= rainy::annotations::moon::no_prefix]]
    std::string b;

    [[
        = rainy::annotations::moon::rename("c_s"),
        = rainy::annotations::moon::with_prefix("type_"),
        = rainy::annotations::moon::use_name_style(annotations::moon::named_style::all_caps_no_underline)
    ]]
    char c;

    std::vector<int> d;
};

// clang-format on

#endif

int main() {
#if RAINY_HAS_CXX26_STATIC_REFLECTION && RAINY_HAS_CXX26
    {
        auto tuple = meta::moon::struct_to_tuple<type>();
        std::println(stdout, "type of member count: {}",meta::moon::member_count_v<type>);
        std::println(stdout, "type of member names: ");
        for (const auto item: meta::moon::get_member_names<type>()) {
            std::print(stdout, "{} ", item.data());
        }
        return 0;
    }
    constexpr bool is_enum_value = meta::moon::is_enum_value_v<aaa::enum_type, aaa::enum_type::a>;
    constexpr bool is_enum_value1 = meta::moon::is_enum_value_v<aaa::enum_type, static_cast<aaa::enum_type>(1)>;
    std::cout << "is_enum_value: " << is_enum_value << '\n';
    std::cout << "is_enum_value1: " << is_enum_value1 << '\n';
    std::cout << "count: " << meta::moon::enum_count<aaa::enum_type>() << '\n';
    std::cout << "foundation::ctti::variable_name<111>(): " << foundation::ctti::variable_name<111>() << '\n';
    std::cout << "foundation::ctti::variable_name<aaa::enum_type::a>(): " << foundation::ctti::variable_name<aaa::enum_type::a>()
              << '\n';
    std::cout << "foundation::ctti::variable_name<static_cast<aaa::enum_type>(1)>(): "
              << foundation::ctti::variable_name<static_cast<aaa::enum_type>(1)>() << '\n';


#endif

    rainy::foundation::io::io_context ctx;
    rainy::foundation::io::steady_timer timer(ctx, std::chrono::seconds{5});
    std::cout << "foundation::ctti::variable_name<aaa::enum_type::a>(): " << foundation::ctti::variable_name<aaa::enum_type::a>() << '\n';
    std::cout << "foundation::ctti::variable_name<static_cast<aaa::enum_type>(1)>(): " << foundation::ctti::variable_name<static_cast<aaa::enum_type>(1)>() << '\n';
    timer.async_wait([](std::error_code ec) {
        if (!ec) {
            std::cout << "!!!\n";
        }
    });
    timer.async_wait([](std::error_code ec) {
        if (!ec) {
            std::cout << "!!!!\n";
        }
    });
    ctx.run();
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
    char buf[200]{};
    foundation::text::to_chars(buf, buf + 10, 11111);
    std::cout << buf << '\n';

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
