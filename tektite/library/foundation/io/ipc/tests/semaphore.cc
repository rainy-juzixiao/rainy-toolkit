#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/ipc/semaphore.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

using namespace rainy::foundation::io;
using namespace rainy::foundation::io::ipc::semaphore;

namespace {
    io_context &global_ctx() {
        static io_context ctx;
        return ctx;
    }

    constexpr auto SEM_NAME = "/rainy_test_sem";
    constexpr auto SEM_NAME_2 = "/rainy_test_sem_2";

    struct test_cleanup {
        ~test_cleanup() {
            semaphore::unlink(SEM_NAME);
            semaphore::unlink(SEM_NAME_2);
        }
    };
}

TEST_CASE("semaphore is non-copyable but movable", "[semaphore][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<semaphore>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<semaphore>);
    STATIC_REQUIRE(std::is_move_constructible_v<semaphore>);
    STATIC_REQUIRE(std::is_move_assignable_v<semaphore>);
}

TEST_CASE("semaphore create and is_open", "[semaphore][create]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());
}

TEST_CASE("semaphore create ec overload", "[semaphore][create]") {
    test_cleanup cleanup;
    std::error_code ec;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(sem.is_open());
}

TEST_CASE("semaphore open existing", "[semaphore][open]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());

    auto sem2 = semaphore::open(global_ctx().get_executor(), SEM_NAME);
    REQUIRE(sem2.is_open());
}

TEST_CASE("semaphore open_or_create", "[semaphore][open_or_create]") {
    test_cleanup cleanup;
    semaphore::unlink(SEM_NAME);
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::open_or_create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());
}

TEST_CASE("semaphore post and wait", "[semaphore][post_wait]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 0;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());

    auto ec = sem.post();
    REQUIRE_FALSE(ec);

    ec = sem.wait();
    REQUIRE_FALSE(ec);
}

TEST_CASE("semaphore try_wait with available count", "[semaphore][try_wait]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());

    auto ec = sem.try_wait();
    REQUIRE_FALSE(ec);
}

TEST_CASE("semaphore try_wait when unavailable", "[semaphore][try_wait]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 0;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());

    auto ec = sem.try_wait();
    REQUIRE(ec);
}

TEST_CASE("semaphore timed_wait with available count", "[semaphore][timed_wait]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());

    auto ec = sem.timed_wait(std::chrono::milliseconds(100));
    REQUIRE_FALSE(ec);
}

TEST_CASE("semaphore timed_wait timeout", "[semaphore][timed_wait]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 0;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());

    auto ec = sem.timed_wait(std::chrono::milliseconds(10));
    REQUIRE(ec);
}

TEST_CASE("semaphore close", "[semaphore][close]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());
    auto ec = sem.close();
    REQUIRE_FALSE(ec);
    REQUIRE_FALSE(sem.is_open());
}

TEST_CASE("semaphore move construction", "[semaphore][move]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());
    auto sem2 = std::move(sem);
    REQUIRE(sem2.is_open());
    REQUIRE_FALSE(sem.is_open());
}

TEST_CASE("semaphore move assignment", "[semaphore][move]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    auto sem2 = semaphore::create(global_ctx().get_executor(), SEM_NAME_2, attr);
    sem2 = std::move(sem);
    REQUIRE(sem2.is_open());
    REQUIRE_FALSE(sem.is_open());
}

TEST_CASE("semaphore swap", "[semaphore][swap]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    auto sem2 = semaphore::create(global_ctx().get_executor(), SEM_NAME_2, attr);
    sem.swap(sem2);
    REQUIRE(sem.is_open());
    REQUIRE(sem2.is_open());
}

TEST_CASE("semaphore unlink", "[semaphore][unlink]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
    REQUIRE(sem.is_open());

    auto ec = semaphore::unlink(SEM_NAME);
    REQUIRE_FALSE(ec);
}

TEST_CASE("semaphore native_handle", "[semaphore][native_handle]") {
    test_cleanup cleanup;
    attributes attr;
    attr.initial_value = 1;
    attr.max_value = 1;
    auto sem = semaphore::create(global_ctx().get_executor(), SEM_NAME, attr);
#if RAINY_USING_WINDOWS
    REQUIRE(sem.native_handle() != reinterpret_cast<semaphore::native_handle_type>(-1));
#else
    REQUIRE(sem.native_handle() != static_cast<semaphore::native_handle_type>(-1));
#endif
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)