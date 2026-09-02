/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <rainy/core/concurrency/tss_ptr.hpp>
#include <rainy/foundation/concurrency/rcu.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/concurrency/thread.hpp>

namespace rainy::foundation::concurrency::implements {
    constexpr std::uint64_t rcu_ctr_invalid = ~std::uint64_t{0};

    struct reader_node {
        core::concurrency::atomic<std::uint64_t> ctr{rcu_ctr_invalid};
        reader_node *next{nullptr};
        std::uint32_t nesting{0};
    };

    struct rcu_domain_state {
        core::concurrency::atomic<std::uint64_t> global_counter{0};
        reader_node *head{nullptr};
        core::concurrency::tss_ptr<reader_node> readers;
        mutex registry_mutex;
    };

    reader_node *get_reader_node(rcu_domain_state *state) {
        auto *node = static_cast<reader_node *>(state->readers);
        if (!node) {
            node = new reader_node;
            state->readers = node;
            state->registry_mutex.lock();
            node->next = state->head;
            state->head = node;
            state->registry_mutex.unlock();
        }
        return node;
    }

    void wait_for_readers(rcu_domain_state *state, const std::uint64_t target) {
        for (reader_node *node = state->head; node; node = node->next) {
            while (true) {
                const auto ctr = node->ctr.load(core::layer::memory_order_acquire);
                if (ctr == rcu_ctr_invalid || ctr == target) {
                    break;
                }
                foundation::system::this_thread::yield();
            }
        }
    }
}

namespace rainy::foundation::concurrency {
    rcu_domain::rcu_domain() noexcept : state_{nullptr} {
        state_ = new (std::nothrow) implements::rcu_domain_state;
        if (!state_) {
            std::terminate();
        }
    }

    rcu_domain::~rcu_domain() {
        delete state_;
    }

    rain_fn rcu_domain::lock() noexcept -> void {
        auto *node = implements::get_reader_node(state_);
        if (node->nesting++ == 0) {
            node->ctr.store(state_->global_counter.load(core::layer::memory_order_seq_cst),
                            core::layer::memory_order_seq_cst);
        }
    }

    RAINY_NODISCARD rain_fn rcu_domain::try_lock() noexcept -> bool {
        lock();
        return true;
    }

    rain_fn rcu_domain::unlock() noexcept -> void {
        auto *node = static_cast<implements::reader_node *>(state_->readers);
        if (--node->nesting == 0) {
            node->ctr.store(implements::rcu_ctr_invalid, core::layer::memory_order_release);
        }
    }

    RAINY_TOOLKIT_API rain_fn rcu_default_domain() noexcept -> rcu_domain & {
        static rcu_domain domain;
        return domain;
    }

    RAINY_TOOLKIT_API rain_fn rcu_synchronize(rcu_domain &dom) noexcept -> void {
        auto *state = dom.state_;
        state->registry_mutex.lock();
        const std::uint64_t base = state->global_counter.load(core::layer::memory_order_seq_cst);
        for (std::uint32_t pass = 1; pass <= 2; ++pass) {
            state->global_counter.store(base + pass, core::layer::memory_order_seq_cst);
            implements::wait_for_readers(state, base + pass);
        }
        state->registry_mutex.unlock();
    }

    RAINY_TOOLKIT_API rain_fn rcu_barrier(rcu_domain &dom) noexcept -> void {
        rcu_synchronize(dom);
    }
}
