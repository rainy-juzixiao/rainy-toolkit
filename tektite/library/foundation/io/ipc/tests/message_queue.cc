#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/ipc/message_queue.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using namespace rainy::foundation::io;
using namespace rainy::foundation::io::ipc::message_queue;

namespace {
    io_context &global_ctx() {
        static io_context ctx;
        return ctx;
    }

    constexpr std::string_view QUEUE_NAME = "/rainy_test_mq";
    constexpr std::string_view PAYLOAD = "message_queue_test";
    constexpr std::size_t BUF_SIZE = 4096;

    struct test_cleanup {
        ~test_cleanup() {
            message_queue::unlink(QUEUE_NAME);
        }
    };

    std::string recv_buffer() {
        return std::string(BUF_SIZE, '\0');
    }
}

TEST_CASE("message_queue is non-copyable but movable", "[message_queue][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<message_queue>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<message_queue>);
    STATIC_REQUIRE(std::is_move_constructible_v<message_queue>);
    STATIC_REQUIRE(std::is_move_assignable_v<message_queue>);
}

TEST_CASE("message_queue create and is_open", "[message_queue][create]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());
    REQUIRE(mq.empty());
    REQUIRE(mq.capacity() == 10);
    REQUIRE(mq.max_message_size() == 4096);
}

TEST_CASE("message_queue create ec overload", "[message_queue][create]") {
    test_cleanup cleanup;
    std::error_code ec;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr, direction::inout, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(mq.is_open());
}

TEST_CASE("message_queue open existing queue", "[message_queue][open]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());

    auto mq2 = message_queue::open(global_ctx().get_executor(), QUEUE_NAME);
    REQUIRE(mq2.is_open());
}

TEST_CASE("message_queue open_or_create creates if not exists", "[message_queue][open_or_create]") {
    test_cleanup cleanup;
    message_queue::unlink(QUEUE_NAME);
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::open_or_create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());
}

TEST_CASE("message_queue unlink removes queue", "[message_queue][unlink]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());

    auto ec = message_queue::unlink(QUEUE_NAME);
    REQUIRE_FALSE(ec);
}

TEST_CASE("message_queue push and pop round-trip", "[message_queue][sync]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());

    auto buf = const_buffer(PAYLOAD.data(), PAYLOAD.size());
    REQUIRE_NOTHROW(mq.push(buf));

    REQUIRE_FALSE(mq.empty());
    REQUIRE(mq.size() == 1);

    std::string recv(attr.max_message_size, '\0');
    auto rbuf = mutable_buffer(recv.data(), recv.size());
    priority prio = priority::low;
    auto n = mq.pop(rbuf, prio);
    REQUIRE(n == PAYLOAD.size());
    REQUIRE(recv.substr(0, n) == PAYLOAD);
    REQUIRE(mq.empty());
}

TEST_CASE("message_queue push with priority and pop receives priority", "[message_queue][priority]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());

    auto buf1 = const_buffer("low", 3);
    auto buf2 = const_buffer("high", 4);
    mq.push(buf1, priority::low);
    mq.push(buf2, priority::high);

    std::string recv(attr.max_message_size, '\0');
    priority prio = priority::low;
    auto n = mq.pop(mutable_buffer(recv.data(), recv.size()), prio);
    REQUIRE(n > 0);
}

TEST_CASE("message_queue push with ec overload", "[message_queue][sync]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    std::error_code ec;
    auto buf = const_buffer(PAYLOAD.data(), PAYLOAD.size());
    mq.push(buf, ec);
    REQUIRE_FALSE(ec);
}

TEST_CASE("message_queue pop with ec overload", "[message_queue][sync]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    auto buf = const_buffer(PAYLOAD.data(), PAYLOAD.size());
    mq.push(buf);

    std::string recv(attr.max_message_size, '\0');
    priority prio = priority::low;
    std::error_code ec;
    auto n = mq.pop(mutable_buffer(recv.data(), recv.size()), prio, ec);
    REQUIRE_FALSE(ec);
    REQUIRE(n == PAYLOAD.size());
}

TEST_CASE("message_queue try_push and try_pop", "[message_queue][try]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    std::error_code ec;
    auto buf = const_buffer(PAYLOAD.data(), PAYLOAD.size());
    REQUIRE(mq.try_push(buf, priority::low, ec));
    REQUIRE_FALSE(ec);

    std::string recv(attr.max_message_size, '\0');
    priority prio = priority::low;
    message_queue::size_type received = 0;
    REQUIRE(mq.try_pop(mutable_buffer(recv.data(), recv.size()), prio, received, ec));
    REQUIRE_FALSE(ec);
    REQUIRE(received == PAYLOAD.size());
    REQUIRE(recv.substr(0, received) == PAYLOAD);
}

TEST_CASE("message_queue timed_push and timed_pop", "[message_queue][timed]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    std::error_code ec;
    auto buf = const_buffer(PAYLOAD.data(), PAYLOAD.size());
    REQUIRE(mq.timed_push(buf, priority::low, std::chrono::milliseconds{100}, ec));
    REQUIRE_FALSE(ec);

    std::string recv(attr.max_message_size, '\0');
    priority prio = priority::low;
    message_queue::size_type received = 0;
    REQUIRE(mq.timed_pop(mutable_buffer(recv.data(), recv.size()), prio, received, std::chrono::milliseconds{100}, ec));
    REQUIRE_FALSE(ec);
    REQUIRE(received == PAYLOAD.size());
}

TEST_CASE("message_queue move construction", "[message_queue][move]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());
    auto mq2 = std::move(mq);
    REQUIRE(mq2.is_open());
    REQUIRE_FALSE(mq.is_open());
}

TEST_CASE("message_queue move assignment", "[message_queue][move]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    auto mq2 = message_queue::create(global_ctx().get_executor(), std::string{QUEUE_NAME} + "_2", attr);
    mq2 = std::move(mq);
    REQUIRE(mq2.is_open());
    REQUIRE_FALSE(mq.is_open());
    message_queue::unlink(std::string{QUEUE_NAME} + "_2");
}

TEST_CASE("message_queue close", "[message_queue][close]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    REQUIRE(mq.is_open());
    auto ec = mq.close();
    REQUIRE_FALSE(ec);
    REQUIRE_FALSE(mq.is_open());
}

TEST_CASE("message_queue native_handle is valid after create", "[message_queue][native_handle]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
#if RAINY_USING_WINDOWS
    REQUIRE(mq.native_handle() != reinterpret_cast<message_queue::native_handle_type>(-1));
#else
    REQUIRE(mq.native_handle() != static_cast<message_queue::native_handle_type>(-1));
#endif
}

TEST_CASE("message_queue swap", "[message_queue][swap]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    auto mq2 = message_queue::create(global_ctx().get_executor(), std::string{QUEUE_NAME} + "_2", attr);
    mq.swap(mq2);
    message_queue::unlink(std::string{QUEUE_NAME} + "_2");
}

TEST_CASE("message_queue get_executor is accessible", "[message_queue][executor]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    (void) mq.get_executor();
    SUCCEED();
}

TEST_CASE("message_queue multiple messages round-trip", "[message_queue][sync]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    constexpr int COUNT = 5;
    for (int i = 0; i < COUNT; ++i) {
        std::string msg = "msg_" + std::to_string(i);
        mq.push(const_buffer(msg.data(), msg.size()));
    }
    REQUIRE(mq.size() == COUNT);
    for (int i = 0; i < COUNT; ++i) {
        std::string recv(attr.max_message_size, '\0');
        priority prio = priority::low;
        auto n = mq.pop(mutable_buffer(recv.data(), recv.size()), prio);
        REQUIRE(n > 0);
    }
    REQUIRE(mq.empty());
}

TEST_CASE("message_queue async_push then async_pop round-trip", "[message_queue][async]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
    attr.max_message_size = 4096;
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);

    std::atomic<bool> push_done{false};
    std::error_code push_ec{};
    std::size_t push_bytes{0};

    auto buf = const_buffer(PAYLOAD.data(), PAYLOAD.size());
    mq.async_push(buf, priority::low, [&](std::error_code ec, std::size_t n) {
        push_ec = ec;
        push_bytes = n;
        push_done.store(true, std::memory_order_release);
    });

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{5};
    while (!push_done.load(std::memory_order_acquire)) {
        if (std::chrono::steady_clock::now() >= deadline) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    REQUIRE(push_done.load());
    REQUIRE_FALSE(push_ec);
    REQUIRE(push_bytes == PAYLOAD.size());

    std::atomic<bool> pop_done{false};
    std::error_code pop_ec{};
    std::size_t pop_bytes{0};
    std::string recv(attr.max_message_size, '\0');

    auto rbuf = mutable_buffer(recv.data(), recv.size());
    mq.async_pop(rbuf, [&](std::error_code ec, std::size_t n) {
        pop_ec = ec;
        pop_bytes = n;
        pop_done.store(true, std::memory_order_release);
    });

    deadline = std::chrono::steady_clock::now() + std::chrono::seconds{5};
    while (!pop_done.load(std::memory_order_acquire)) {
        if (std::chrono::steady_clock::now() >= deadline) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    REQUIRE(pop_done.load());
    REQUIRE_FALSE(pop_ec);
    REQUIRE(pop_bytes == PAYLOAD.size());
    REQUIRE(recv.substr(0, pop_bytes) == PAYLOAD);
}

TEST_CASE("message_queue large message round-trip", "[message_queue][sync]") {
    test_cleanup cleanup;
    attributes attr;
    attr.max_messages = 10;
#if RAINY_USING_MACOS
    attr.max_message_size = 1000;
#else
    attr.max_message_size = 4096;
#endif
    auto mq = message_queue::create(global_ctx().get_executor(), QUEUE_NAME, attr);
    std::string large(attr.max_message_size, 'L');
    mq.push(const_buffer(large.data(), large.size()));

    std::string recv(4096, '\0');
    priority prio = priority::low;
    auto n = mq.pop(mutable_buffer(recv.data(), recv.size()), prio);
    REQUIRE(n == large.size());
    REQUIRE(recv.substr(0, n) == large);
}