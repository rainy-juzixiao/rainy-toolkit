#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/ipc/shared_memory.hpp>
#include <cstring>
#include <string>
#include <string_view>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

using namespace rainy::foundation::io;
using namespace rainy::foundation::io::ipc::shared_memory;

namespace {
    io_context &global_ctx() {
        static io_context ctx;
        return ctx;
    }

    constexpr std::string_view SHM_NAME = "/rainy_test_shm";
    constexpr std::size_t SHM_SIZE = 4096;

    struct test_cleanup {
        ~test_cleanup() {
            shared_memory::unlink(SHM_NAME);
        }
    };
}

TEST_CASE("shared_memory is non-copyable but movable", "[shared_memory][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<shared_memory>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<shared_memory>);
    STATIC_REQUIRE(std::is_move_constructible_v<shared_memory>);
    STATIC_REQUIRE(std::is_move_assignable_v<shared_memory>);
}

TEST_CASE("shared_memory create and is_open", "[shared_memory][create]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());
    REQUIRE(shm.size() == SHM_SIZE);
    REQUIRE_FALSE(shm.empty());
    REQUIRE(shm.data() != nullptr);
}

TEST_CASE("shared_memory create ec overload", "[shared_memory][create]") {
    test_cleanup cleanup;
    std::error_code ec;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(shm.is_open());
}

TEST_CASE("shared_memory open existing", "[shared_memory][open]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());

    auto shm2 = shared_memory::open(global_ctx().get_executor(), SHM_NAME);
    REQUIRE(shm2.is_open());
    REQUIRE(shm2.size() > 0);
    REQUIRE(shm2.data() != nullptr);
}

TEST_CASE("shared_memory open_or_create", "[shared_memory][open_or_create]") {
    test_cleanup cleanup;
    shared_memory::unlink(SHM_NAME);
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::open_or_create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());
}

TEST_CASE("shared_memory write and read", "[shared_memory][rw]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());

    const char *message = "hello shared memory";
    std::memcpy(shm.data(), message, std::strlen(message) + 1);

    auto shm2 = shared_memory::open(global_ctx().get_executor(), SHM_NAME);
    REQUIRE(shm2.is_open());
    REQUIRE(std::memcmp(shm2.data(), message, std::strlen(message) + 1) == 0);
}

TEST_CASE("shared_memory flush", "[shared_memory][flush]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());

    std::memset(shm.data(), 0xAB, SHM_SIZE);
    auto ec = shm.flush();
    REQUIRE_FALSE(ec);
}

TEST_CASE("shared_memory flush range", "[shared_memory][flush]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());

    std::memset(shm.data(), 0xCD, 256);
    auto ec = shm.flush(0, 256);
    REQUIRE_FALSE(ec);
}

TEST_CASE("shared_memory close", "[shared_memory][close]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());
    auto ec = shm.close();
    REQUIRE_FALSE(ec);
    REQUIRE_FALSE(shm.is_open());
}

TEST_CASE("shared_memory move construction", "[shared_memory][move]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());
    auto shm2 = std::move(shm);
    REQUIRE(shm2.is_open());
    REQUIRE_FALSE(shm.is_open());
}

TEST_CASE("shared_memory move assignment", "[shared_memory][move]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    auto shm2 = shared_memory::create(global_ctx().get_executor(), std::string{SHM_NAME} + "_2", attr);
    shm2 = std::move(shm);
    REQUIRE(shm2.is_open());
    REQUIRE_FALSE(shm.is_open());
    shared_memory::unlink(std::string{SHM_NAME} + "_2");
}

TEST_CASE("shared_memory swap", "[shared_memory][swap]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    auto shm2 = shared_memory::create(global_ctx().get_executor(), std::string{SHM_NAME} + "_2", attr);
    shm.swap(shm2);
    REQUIRE(shm.is_open());
    REQUIRE(shm2.is_open());
    shared_memory::unlink(std::string{SHM_NAME} + "_2");
}

TEST_CASE("shared_memory unlink", "[shared_memory][unlink]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
    REQUIRE(shm.is_open());

    auto ec = shared_memory::unlink(SHM_NAME);
    REQUIRE_FALSE(ec);
}

TEST_CASE("shared_memory native_handle", "[shared_memory][native_handle]") {
    test_cleanup cleanup;
    attributes attr;
    attr.size = SHM_SIZE;
    attr.mode = access_mode::read_write;
    auto shm = shared_memory::create(global_ctx().get_executor(), SHM_NAME, attr);
#if RAINY_USING_WINDOWS
    REQUIRE(shm.native_handle() != reinterpret_cast<shared_memory::native_handle_type>(-1));
#else
    REQUIRE(shm.native_handle() != static_cast<shared_memory::native_handle_type>(-1));
#endif
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)