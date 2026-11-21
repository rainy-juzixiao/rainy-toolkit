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
#include <cstdint>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/layer.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/container/implements/singleton/registry.hpp>

namespace rainy::foundation::container::implements {
    struct singleton_registry::impl {
        struct node {
            entry value;
            node *next;
        };

        struct pool_block {
            void *pointer;
            std::size_t size;
            std::size_t align;
        };

        node *entries{nullptr};
        pool_block *blocks{nullptr};
        std::size_t block_count{0};
        std::size_t block_capacity{0};
        concurrency::mutex mutex;

        void grow_blocks() {
            if (block_count < block_capacity) {
                return;
            }
            const std::size_t next_capacity = block_capacity == 0 ? 8 : block_capacity * 2;
            auto *grown = new pool_block[next_capacity];
            for (std::size_t i = 0; i < block_count; ++i) {
                grown[i] = blocks[i];
            }
            delete[] blocks;
            blocks = grown;
            block_capacity = next_capacity;
        }
    };

    singleton_registry::singleton_registry() : global_ptr_(new impl{}) {
    }

    singleton_registry::~singleton_registry() {
        impl *state = global_ptr_;
        if (state == nullptr) {
            return;
        }
        for (impl::node *curr = state->entries; curr != nullptr;) {
            impl::node *next = curr->next;
            if (curr->value.destroy != nullptr) {
                curr->value.destroy(curr->value.block);
            }
            delete curr;
            curr = next;
        }
        for (std::size_t i = 0; i < state->block_count; ++i) {
            core::layer::deallocate(state->blocks[i].pointer, state->blocks[i].size, state->blocks[i].align);
        }
        delete[] state->blocks;
        delete state;
        global_ptr_ = nullptr;
    }

    singleton_registry &singleton_registry::instance() {
        static singleton_registry registry;
        return registry;
    }

    void singleton_registry::lock() noexcept {
        global_ptr_->mutex.lock();
    }

    void singleton_registry::unlock() noexcept {
        global_ptr_->mutex.unlock();
    }

    void *singleton_registry::find_locked(const std::size_t hash, const core::typeinfo *type) const noexcept {
        for (impl::node *curr = global_ptr_->entries; curr != nullptr; curr = curr->next) {
            if (curr->value.hash == hash && *curr->value.type == *type) {
                return curr->value.block;
            }
        }
        return nullptr;
    }

    void *singleton_registry::allocate_locked(const std::size_t size, const std::size_t align) {
        const std::size_t alignment = align < alignof(std::max_align_t) ? alignof(std::max_align_t) : align;
        const std::size_t total = size + alignment;
        impl *state = global_ptr_;
        state->grow_blocks();
        void *raw = core::layer::allocate(total, alignment);
        if (raw == nullptr) {
            core::exceptions::runtime::throw_bad_alloc();
        }
        state->blocks[state->block_count++] = impl::pool_block{raw, total, alignment};
        const auto address = reinterpret_cast<std::uintptr_t>(raw);
        const auto mask = static_cast<std::uintptr_t>(alignment) - 1;
        return reinterpret_cast<void *>((address + mask) & ~mask);
    }

    void singleton_registry::record_locked(const std::size_t hash, const core::typeinfo *type, void *block, void (*destroy)(void *)) {
        auto *node = new impl::node{entry{hash, type, block, destroy}, global_ptr_->entries};
        global_ptr_->entries = node;
    }
}
