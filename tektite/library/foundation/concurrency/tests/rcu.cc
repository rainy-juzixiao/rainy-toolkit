#include <catch2/catch_test_macros.hpp>

#include <rainy/foundation/concurrency/rcu.hpp>
#include <rainy/foundation/concurrency/thread.hpp>

using namespace rainy::foundation::concurrency;
using rainy::core::concurrency::atomic;

namespace {
    struct rcu_test_data {
        int value{0};
        atomic<int> *delete_counter{nullptr};

        rcu_test_data() = default;

        explicit rcu_test_data(const int v) : value{v} {
        }
    };

    struct counting_deleter {
        template <typename Ty>
        void operator()(Ty *p) const {
            if (p->delete_counter) {
                p->delete_counter->fetch_add(1, rainy::core::layer::memory_order_relaxed);
            }
            delete p;
        }
    };
}

TEST_CASE("rcu_domain basic lockable behavior", "[rcu][domain]") {
    rcu_domain dom;

    SECTION("lock and unlock establish a region") {
        REQUIRE_NOTHROW(dom.lock());
        REQUIRE_NOTHROW(dom.unlock());
    }

    SECTION("try_lock always succeeds and opens a region") {
        REQUIRE(dom.try_lock());
        REQUIRE_NOTHROW(dom.unlock());
    }

    SECTION("nested regions are paired") {
        dom.lock();
        dom.lock();
        dom.unlock();
        dom.unlock();
    }

    SECTION("try_lock can be used nested with lock") {
        dom.lock();
        REQUIRE(dom.try_lock());
        dom.unlock();
        dom.unlock();
    }
}

TEST_CASE("rcu_default_domain returns the same object", "[rcu][domain]") {
    auto &first = rcu_default_domain();
    auto &second = rcu_default_domain();
    REQUIRE(&first == &second);
}

TEST_CASE("rcu_synchronize waits for pre-existing readers", "[rcu][synchronize]") {
    rcu_domain dom;
    atomic<bool> reader_entered{false};
    atomic<bool> reader_should_exit{false};
    atomic<bool> synchronize_returned{false};

    thread reader{[&] {
        dom.lock();
        reader_entered.store(true, rainy::core::layer::memory_order_release);
        while (!reader_should_exit.load(rainy::core::layer::memory_order_acquire)) {
            rainy::foundation::concurrency::this_thread::yield();
        }
        dom.unlock();
    }};

    while (!reader_entered.load(rainy::core::layer::memory_order_acquire)) {
        rainy::foundation::concurrency::this_thread::yield();
    }

    thread synchronizer{[&] {
        rcu_synchronize(dom);
        synchronize_returned.store(true, rainy::core::layer::memory_order_release);
    }};

    // 读侧仍持有保护区域，synchronize 不应返回
    this_thread::yield();
    REQUIRE_FALSE(synchronize_returned.load(rainy::core::layer::memory_order_acquire));

    reader_should_exit.store(true, rainy::core::layer::memory_order_release);
    synchronizer.join();
    REQUIRE(synchronize_returned.load(rainy::core::layer::memory_order_acquire));
    reader.join();
}

TEST_CASE("rcu_synchronize does not wait for later readers", "[rcu][synchronize]") {
    rcu_domain dom;
    atomic<bool> late_reader_done{false};

    rcu_synchronize(dom);

    thread late_reader{[&] {
        dom.lock();
        late_reader_done.store(true, rainy::core::layer::memory_order_release);
        dom.unlock();
    }};
    late_reader.join();
    REQUIRE(late_reader_done.load(rainy::core::layer::memory_order_acquire));
}

TEST_CASE("rcu_retire defers reclamation until safe", "[rcu][retire]") {
    rcu_domain dom;
    atomic<int> delete_count{0};

    rcu_test_data *p = new rcu_test_data(42);
    p->delete_counter = &delete_count;

    {
        dom.lock();
        REQUIRE(p->value == 42);
        dom.unlock();
    }

    rcu_retire(p, counting_deleter{}, dom);
    REQUIRE(delete_count.load(rainy::core::layer::memory_order_acquire) == 1);
}

TEST_CASE("rcu_retire with default deleter", "[rcu][retire]") {
    rcu_domain dom;
    static atomic<int> plain_delete_count{0};

    struct plain_data {
        ~plain_data() {
            plain_delete_count.fetch_add(1, rainy::core::layer::memory_order_relaxed);
        }
    };

    plain_data *p = new plain_data;
    rcu_retire(p, rainy::core::memory::default_deleter<plain_data>{}, dom);
    REQUIRE(plain_delete_count.load(rainy::core::layer::memory_order_acquire) == 1);
}

TEST_CASE("rcu_obj_base retire reclaims the object", "[rcu][obj_base]") {
    rcu_domain dom;
    atomic<int> delete_count{0};

    struct intruded_data : rcu_obj_base<intruded_data, counting_deleter> {
        int value{0};
        atomic<int> *delete_counter{nullptr};
    };

    intruded_data *p = new intruded_data;
    p->value = 7;
    p->delete_counter = &delete_count;

    {
        dom.lock();
        REQUIRE(p->value == 7);
        dom.unlock();
    }

    p->retire(counting_deleter{}, dom);
    REQUIRE(delete_count.load(rainy::core::layer::memory_order_acquire) == 1);
}

TEST_CASE("rcu_barrier waits for queued reclamation", "[rcu][barrier]") {
    rcu_domain dom;
    atomic<int> delete_count{0};

    rcu_test_data *p = new rcu_test_data(1);
    p->delete_counter = &delete_count;
    rcu_retire(p, counting_deleter{}, dom);

    rcu_barrier(dom);
    REQUIRE(delete_count.load(rainy::core::layer::memory_order_acquire) == 1);
}

TEST_CASE("rcu read-copy-update concurrent stress", "[rcu][stress]") {
    rcu_domain dom;
    atomic<int> delete_count{0};
    atomic<rcu_test_data *> shared{nullptr};
    atomic<bool> done{false};

    shared.store(new rcu_test_data(0), rainy::core::layer::memory_order_relaxed);
    shared.load(rainy::core::layer::memory_order_relaxed)->delete_counter = &delete_count;

    constexpr int reader_count = 4;
    thread readers[reader_count];
    for (int i = 0; i < reader_count; ++i) {
        readers[i] = thread{[&] {
            while (!done.load(rainy::core::layer::memory_order_acquire)) {
                dom.lock();
                rcu_test_data *p = shared.load(rainy::core::layer::memory_order_acquire);
                // 读者只能依赖保护区域保证 p 的有效性
                volatile int v = p->value; // NOLINT
                (void) v;
                dom.unlock();
            }
        }};
    }

    for (int i = 1; i <= 100; ++i) {
        rcu_test_data *next = new rcu_test_data(i);
        next->delete_counter = &delete_count;
        rcu_test_data *old = shared.exchange(next, rainy::core::layer::memory_order_acq_rel);
        rcu_retire(old, counting_deleter{}, dom);
    }

    done.store(true, rainy::core::layer::memory_order_release);
    for (int i = 0; i < reader_count; ++i) {
        readers[i].join();
    }

    rcu_retire(shared.load(rainy::core::layer::memory_order_acquire), counting_deleter{}, dom);
    REQUIRE(delete_count.load(rainy::core::layer::memory_order_acquire) == 101);
}
