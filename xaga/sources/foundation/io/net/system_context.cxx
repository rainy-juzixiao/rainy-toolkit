#include <rainy/foundation/io/net/executor/system_context.hpp>
#include <rainy/foundation/io/net/executor/system_executor.hpp>

namespace rainy::foundation::io::net {
    struct system_context::thread_function {
        concurrency::executor* exec_;

        void operator()() const {
            exec_->wait_all();
        }
    };

    system_context::system_context()
        : exec_(concurrency::make_pooled_executor()) {
    }

    system_context::~system_context() {
        stop();
        join();
    }

    system_context::executor_type system_context::get_executor() noexcept {
        return executor_type(*this);
    }

    void system_context::stop() {
        exec_.stop();
    }

    bool system_context::stopped() const noexcept {
        return exec_.stopped();
    }

    void system_context::join() {
        exec_.join();
    }
}