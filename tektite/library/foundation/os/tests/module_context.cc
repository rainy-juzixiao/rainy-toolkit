#include <rainy/foundation/os/module_context.hpp>
#include <catch2/catch_all.hpp>

#include <string>

using namespace rainy::foundation;

namespace {
#ifdef _WIN32
    constexpr const char *system_library = "ntdll.dll";
    constexpr const char *exported_symbol = "RtlCaptureContext";
#else
    constexpr const char *system_library = "libm.so.6";
    constexpr const char *exported_symbol = "cos";
#endif
    constexpr const char *missing_library = "definitely_not_a_real_module_42";
    constexpr const char *missing_symbol = "definitely_not_a_real_symbol_42";
}

TEST_CASE("module_context load and release", "[module_context]") {
    dynamic_library::module_context context(system_library, false);
    REQUIRE(context.is_loaded());
    REQUIRE(context.native_handle() != nullptr);

    REQUIRE(context.release());
    REQUIRE_FALSE(context.is_loaded());
    REQUIRE(context.native_handle() == nullptr);
}

TEST_CASE("module_context load symbol", "[module_context]") {
    dynamic_library::module_context context(system_library, false);
    REQUIRE(context.is_loaded());

    SECTION("load_symbol") {
        auto symbol = context.load_symbol(exported_symbol);
        REQUIRE(symbol != nullptr);
        REQUIRE(context.load_symbol(missing_symbol) == nullptr);
    }
    SECTION("load_symbol_to") {
        void (*symbol)() = nullptr;
        REQUIRE(context.load_symbol_to(exported_symbol, symbol));
        REQUIRE(symbol != nullptr);
    }
}

TEST_CASE("module_context missing module", "[module_context]") {
    dynamic_library::module_context context(missing_library, false);
    REQUIRE_FALSE(context.is_loaded());
    REQUIRE(context.native_handle() == nullptr);
    REQUIRE_FALSE(context.load(missing_library));
    REQUIRE_FALSE(context.is_loaded());
}

TEST_CASE("module_context try_get_module", "[module_context]") {
    dynamic_library::module_context loader(system_library, false);
    REQUIRE(loader.is_loaded());

    dynamic_library::module_context observer(system_library, true);
    REQUIRE(observer.is_loaded());
}
