#include <atomic>
#include <thread>
#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/concurrency/stop_token.hpp>

using namespace rainy::foundation::concurrency;

TEST_CASE("Default constructed stop_token is empty", "[stop_token]") {
    stop_token token;
    stop_token other;
    REQUIRE_FALSE(token.stop_possible());
    REQUIRE_FALSE(token.stop_requested());
    REQUIRE(token == other);
    REQUIRE_FALSE(token != other);
}

TEST_CASE("stop_source basic queries", "[stop_token]") {
    stop_source source;
    REQUIRE_FALSE(source.stop_requested());
    REQUIRE(source.stop_possible());
    REQUIRE_FALSE(source.get_token().stop_requested());
}

TEST_CASE("request_stop returns first-time true then false", "[stop_token]") {
    stop_source source;
    REQUIRE(source.request_stop());
    REQUIRE_FALSE(source.request_stop());
    REQUIRE(source.stop_requested());
}

TEST_CASE("token observes a stop requested by its source", "[stop_token]") {
    stop_source source;
    stop_token token = source.get_token();
    REQUIRE_FALSE(token.stop_requested());
    REQUIRE(source.request_stop());
    REQUIRE(token.stop_requested());
    REQUIRE(source.stop_requested());
}

TEST_CASE("token and source sharing same state compare equal", "[stop_token]") {
    stop_source source;
    stop_token t1 = source.get_token();
    stop_token t2 = source.get_token();
    REQUIRE(t1 == t2);
    REQUIRE_FALSE(t1 != t2);
}

TEST_CASE("stop_callback invoked when stop is requested", "[stop_token]") {
    stop_source source;
    stop_token token = source.get_token();
    std::atomic<bool> called{false};
    {
        stop_callback cb(token, [&called] { called = true; });
        REQUIRE_FALSE(called.load());
        source.request_stop();
    }
    REQUIRE(called.load());
}

TEST_CASE("stop_callback invoked asynchronously in requesting thread", "[stop_token]") {
    stop_source source;
    stop_token token = source.get_token();
    std::atomic<int> count{0};
    std::atomic<std::thread::id> caller_id{};
    std::thread::id expected;
    std::thread requester([&] {
        expected = std::this_thread::get_id();
        stop_callback cb(token, [&] {
            caller_id = std::this_thread::get_id();
            count.fetch_add(1);
        });
        source.request_stop();
    });
    requester.join();
    REQUIRE(count.load() == 1);
    REQUIRE(caller_id.load() == expected);
}

TEST_CASE("stop_callback fires immediately if already requested", "[stop_token]") {
    stop_source source;
    source.request_stop();
    std::atomic<bool> called{false};
    stop_callback cb(source.get_token(), [&called] { called = true; });
    REQUIRE(called.load());
}

TEST_CASE("stop_callback fires immediately for empty token", "[stop_token]") {
    stop_token token;
    std::atomic<bool> called{false};
    stop_callback cb(token, [&called] { called = true; });
    REQUIRE(called.load());
}

TEST_CASE("stop_callback destroyed before request is never invoked", "[stop_token]") {
    stop_source source;
    stop_token token = source.get_token();
    std::atomic<bool> called{false};
    {
        stop_callback cb(token, [&called] { called = true; });
    }
    source.request_stop();
    REQUIRE_FALSE(called.load());
}

TEST_CASE("multiple stop_callbacks all execute", "[stop_token]") {
    stop_source source;
    stop_token token = source.get_token();
    std::atomic<int> count{0};
    {
        stop_callback cb1(token, [&count] { count.fetch_add(1); });
        stop_callback cb2(token, [&count] { count.fetch_add(1); });
        stop_callback cb3(token, [&count] { count.fetch_add(1); });
        source.request_stop();
    }
    REQUIRE(count.load() == 3);
}

TEST_CASE("nostopstate stop_source is inert", "[stop_token]") {
    stop_source source(nostopstate);
    REQUIRE_FALSE(source.stop_possible());
    REQUIRE_FALSE(source.stop_requested());
    REQUIRE_FALSE(source.request_stop());
    stop_token token = source.get_token();
    REQUIRE_FALSE(token.stop_possible());
    REQUIRE_FALSE(token.stop_requested());
}

TEST_CASE("stop_token copy and move semantics", "[stop_token]") {
    stop_source source;
    stop_token original = source.get_token();
    stop_token copied = original;
    REQUIRE(original == copied);
    REQUIRE(copied.stop_possible());
    stop_token moved = std::move(original);
    REQUIRE(moved.stop_possible());
    REQUIRE_FALSE(original.stop_possible());
    REQUIRE_FALSE(moved == original);
}

TEST_CASE("stop_source copy and move semantics", "[stop_token]") {
    stop_source source;
    stop_source copied(source);
    REQUIRE(source == copied);
    copied.request_stop();
    REQUIRE(source.stop_requested());
    stop_source moved(std::move(source));
    REQUIRE(moved.stop_requested());
    REQUIRE_FALSE(source.stop_possible());
}

TEST_CASE("swap exchange tokens and sources", "[stop_token]") {
    stop_source s1;
    stop_source s2;
    stop_token t1 = s1.get_token();
    stop_token t2 = s2.get_token();
    swap(t1, t2);
    s1.request_stop();
    REQUIRE(t2.stop_requested());
    REQUIRE_FALSE(t1.stop_requested());
}