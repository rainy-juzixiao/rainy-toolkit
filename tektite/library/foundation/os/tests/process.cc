#include <catch2/catch_all.hpp>
#include <rainy/foundation/os/process.hpp>

#include <string>
#include <vector>

using namespace rainy::foundation;

namespace {
#ifdef _WIN32
    constexpr const char *exit_program = "cmd";
    std::vector<std::string> exit_arguments() {
        return {"/c", "exit 7"};
    }
#else
    constexpr const char *exit_program = "sh";
    std::vector<std::string> exit_arguments() {
        return {"-c", "exit 7"};
    }
#endif

    rainy::core::collections::vector<rainy::core::text::string> to_rainy_arguments(const std::vector<std::string> &args) {
        rainy::core::collections::vector<rainy::core::text::string> arguments;
        for (const auto &argument : args) {
            arguments.emplace_back(argument);
        }
        return arguments;
    }
}

TEST_CASE("process create and wait", "[process]") {
    os::process process(exit_program, to_rainy_arguments(exit_arguments()));
    REQUIRE(process.id() != 0);

    const int code = process.wait();
    REQUIRE(code == 7);
}

TEST_CASE("process move semantics", "[process]") {
    os::process process(exit_program, to_rainy_arguments(exit_arguments()));
    const auto id = process.id();

    os::process moved(std::move(process));
    REQUIRE(moved.id() == id);
    REQUIRE(process.id() == 0);
    REQUIRE(moved.wait() == 7);
}

TEST_CASE("process list enumerates the current process", "[process]") {
    const auto entries = os::process_list();
    REQUIRE_FALSE(entries.empty());

    const auto self = os::current_process_id();
    bool found = false;
    for (const auto &entry : entries) {
        if (entry.id == self) {
            found = true;
            REQUIRE_FALSE(entry.name.empty());
            break;
        }
    }
    REQUIRE(found);
}

TEST_CASE("process priority", "[process]") {
    const int original = os::current_priority();

#ifdef _WIN32
    const int lowered = 0x00000040; // IDLE_PRIORITY_CLASS
    const int restored = 0x00000020; // NORMAL_PRIORITY_CLASS
#else
    // 非 root 只能调低优先级（增大 nice 值），因此向下调整后应能读回。
    const int lowered = original + 1 <= 19 ? original + 1 : original;
    const int restored = original;
#endif
    REQUIRE(os::set_current_priority(lowered));
    REQUIRE(os::current_priority() == lowered);
    if (restored != lowered) {
        // 恢复原优先级（调高优先级）需要特权，尽力而为即可。
        static_cast<void>(os::set_current_priority(restored));
    }
}
