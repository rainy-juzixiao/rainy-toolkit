#include <catch2/catch_all.hpp>
#include <rainy/core/system.hpp>

using namespace rainy::core;

TEST_CASE("cpu architecture and instruction set", "[system]") {
    SECTION("current_architecture") {
        const auto architecture = system::current_architecture();
        REQUIRE(architecture != system::cpu_architecture::unknown);
#if RAINY_IS_ARM64
        REQUIRE(architecture == system::cpu_architecture::arm64);
#else
        REQUIRE(architecture == system::cpu_architecture::x86_64);
#endif
    }
#if RAINY_IS_ARM64
    SECTION("arm64 hardware capabilities") {
        const bool any = system::has_arm64_feature(system::arm64_feature::asimd) ||
                         system::has_arm64_feature(system::arm64_feature::aes) ||
                         system::has_arm64_feature(system::arm64_feature::crc32);
        REQUIRE(any);
    }
#else
    SECTION("x86 instruction set") {
        REQUIRE(builtin::has_instruction(rainy::core::instruction_set::sse2));
        REQUIRE_FALSE(builtin::get_vendor(nullptr, 0) == 0);
    }
#endif
}

TEST_CASE("memory capacity query", "[system]") {
    const auto capacity = system::query_memory_capacity();
    REQUIRE(capacity.total > 0);
    REQUIRE(capacity.available > 0);
    REQUIRE(capacity.available <= capacity.total);
}

TEST_CASE("environment variables", "[system]") {
    SECTION("set, get and remove") {
        REQUIRE(system::set_environment("RAINY_TEST_VARIABLE", "rainy-value"));
        const auto value = system::get_environment("RAINY_TEST_VARIABLE");
        REQUIRE(value.has_value());
        REQUIRE(*value == "rainy-value");

        REQUIRE(system::remove_environment("RAINY_TEST_VARIABLE"));
        REQUIRE_FALSE(system::get_environment("RAINY_TEST_VARIABLE").has_value());
    }
    SECTION("missing variable yields empty optional") {
        REQUIRE_FALSE(system::get_environment("RAINY_TEST_MISSING_VARIABLE_42").has_value());
    }
}

TEST_CASE("user and group queries", "[system]") {
    SECTION("user name is not empty") {
        const auto name = system::user_name();
        REQUIRE_FALSE(name.empty());
    }
    SECTION("group id and name") {
        const auto group_id = system::current_group_id();
        const auto name = system::group_name(group_id);
        if (name.has_value()) {
            REQUIRE_FALSE(name->empty());
        }
    }
}

TEST_CASE("system random", "[system]") {
    unsigned char first[32]{};
    unsigned char second[32]{};
    REQUIRE(system::system_random(first, sizeof(first)));
    REQUIRE(system::system_random(second, sizeof(second)));
    REQUIRE(std::memcmp(first, second, sizeof(first)) != 0);
    REQUIRE_FALSE(system::system_random(nullptr, 8));
    REQUIRE_FALSE(system::system_random(first, 0));
}
