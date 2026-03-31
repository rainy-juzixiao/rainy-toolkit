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
#include <rainy/foundation/io/net/executor/strand.hpp>
#include <rainy/foundation/concurrency/basic/context.hpp>

namespace rainy::foundation::io::net::implements {
    strand_executor_service::strand_executor_service(execution_context &context) :
        execution_context_service_base(context), impl_list_(nullptr), salt_(0) {
    }

    void strand_executor_service::shutdown() noexcept {
        op_queue<scheduler_operation> ops;
        concurrency::scoped_lock lock(mutex_);

        strand_impl *impl = impl_list_;
        while (impl) {
            impl->mutex_->lock();
            impl->shutdown_ = true;
            ops.push(impl->waiting_queue_);
            ops.push(impl->ready_queue_);
            impl->mutex_->unlock();
            impl = impl->next_;
        }
    }

    strand_executor_service::implementation_type strand_executor_service::create_implementation() {
        implementation_type new_impl(new strand_impl);
        new_impl->locked_ = false;
        new_impl->shutdown_ = false;
        concurrency::scoped_lock lock(mutex_);
        const std::size_t salt = salt_++;
        std::size_t mutex_index = reinterpret_cast<std::size_t>(new_impl.get());
        mutex_index += (reinterpret_cast<std::size_t>(new_impl.get()) >> 3);
        mutex_index ^= salt + 0x9e3779b9 + (mutex_index << 6) + (mutex_index >> 2);
        mutex_index = mutex_index % num_mutexes;
        if (!mutexes_[mutex_index].get()) {
            mutexes_[mutex_index].reset(new concurrency::mutex);
        }
        new_impl->mutex_ = mutexes_[mutex_index].get();
        new_impl->next_ = impl_list_;
        new_impl->prev_ = nullptr;
        if (impl_list_) {
            impl_list_->prev_ = new_impl.get();
        }
        impl_list_ = new_impl.get();
        new_impl->service_ = this;
        return new_impl;
    }

    strand_executor_service::strand_impl::~strand_impl() {
        concurrency::scoped_lock lock(service_->mutex_);
        if (service_->impl_list_ == this) {
            service_->impl_list_ = next_;
        }
        if (prev_) {
            prev_->next_ = next_;
        }
        if (next_) {
            next_->prev_ = prev_;
        }
    }

    bool strand_executor_service::enqueue(const implementation_type &impl, scheduler_operation *op) {
        // NOLINTBEGIN
        impl->mutex_->lock();
        if (impl->shutdown_) {
            impl->mutex_->unlock();
            op->destroy();
            return false;
        } else if (impl->locked_) {
            impl->waiting_queue_.push(op);
            impl->mutex_->unlock();
            return false;
        } else {
            impl->locked_ = true;
            impl->mutex_->unlock();
            impl->ready_queue_.push(op);
            return true;
        }
        // NOLINTEND
    }

    bool strand_executor_service::running_in_this_thread(const implementation_type &impl) {
        return concurrency::implements::call_stack<strand_impl>::contains(impl.get());
    }
}
