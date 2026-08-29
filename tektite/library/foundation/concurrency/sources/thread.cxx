/*
* Copyright 2025 rainy-juzixiao
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
#include <rainy/core/diagnostics/contract.hpp>
#include <rainy/foundation/concurrency/thread.hpp>

namespace rainy::foundation::concurrency {
    thread::~thread() {
        switch (policy_) {
            case policy::auto_join:
                if (joinable()) {
                    join();
                }
                break;
            case policy::auto_detach:
                if (joinable()) {
                    detach();
                }
                break;
            case policy::manual:
            default:
                // manual模式表示正常threading模式，如果使用manual模式
                // 由用户自己确保线程生命周期，如果此处一旦出现内存泄漏，必须进行释放
                // 因此，此处不会允许程序的正常执行
                if (joinable()) {
                    std::terminate();
                }
                break;
        }
    }

    thread &thread::operator=(thread &&right) noexcept {
        core::diagnostics::contracts::expects(!joinable(), "We can't deprecated this thread. it's still exists. unless using detach() or join()");
        this->thread_handle = utility::exchange(right.thread_handle, {});
        this->policy_ = utility::exchange(right.policy_, policy::manual);
        return *this;
    }

    void thread::join() {
        core::diagnostics::contracts::expects(joinable(), "Current Thread Is Invalid");
        core::diagnostics::contracts::expects(get_id() != thread::id{implements::get_thread_id()}, "Resource deadlock would occur");
        auto result = implements::thread_join(thread_handle, nullptr);
        if (result == implements::thrd_result::success) {
            thread_handle = {};
            return;
        }
        switch (result) {
            case implements::thrd_result::busy:
                core::diagnostics::contracts::ensures(false, "Failed to join thread. because [thread_join() - pal] returns busy");
                break;
            case implements::thrd_result::error:
                core::diagnostics::contracts::ensures(
                    false, "Failed to join thread. because [thread_join() - pal] returns error. consider debug this "
                           "function [thread_join() - pal]");
                break;
            default:
                core::diagnostics::contracts::ensures(
                    false,
                    "Failed to join thread. because [thread_join() - pal] has found a unknown error consider debug this "
                    "function [thread_join() - pal]");
                break;
        }
    }

    void thread::detach() {
        core::diagnostics::contracts::expects(joinable(), "Current Thread Is Invalid");
        core::diagnostics::contracts::expects(get_id() != thread::id{implements::get_thread_id()}, "Resource deadlock would occur");
        core::diagnostics::contracts::ensures(implements::thread_detach(thread_handle) == implements::thrd_result::success, "Failed to detach current thread.");
        thread_handle = {};
    }

    bool thread::joinable() const noexcept {
        return static_cast<bool>(thread_handle.handle);
    }

    void thread::sleep_for(const unsigned long ms) noexcept {
        implements::thread_sleep_for(ms);
    }

    RAINY_NODISCARD thread::native_handle_type thread::native_handle() const noexcept {
        return reinterpret_cast<native_handle_type>(thread_handle.handle);
    }

    void thread::suspend() noexcept {
        implements::suspend_thread(this->thread_handle);
    }

    void thread::resume() noexcept {
        implements::resume_thread(this->thread_handle);
    }

    thread::id thread::get_id() const noexcept {
#if RAINY_USING_LINUX
        return thread::id{this->thread_handle.handle};
#else
        return thread::id{this->thread_handle.tid};
#endif
    }

    bool operator==(thread::id left, thread::id right) noexcept {
        return left.id_ == right.id_;
    }

#if !RAINY_HAS_CXX20
    RAINY_NODISCARD bool operator!=(thread::id left, thread::id right) noexcept {
        return !(left == right);
    }

    RAINY_NODISCARD bool operator<(thread::id left, thread::id right) noexcept {
        return left.id_ < right.id_;
    }

    RAINY_NODISCARD bool operator<=(thread::id left, thread::id right) noexcept {
        return !(right < left);
    }

    RAINY_NODISCARD bool operator>(thread::id left, thread::id right) noexcept {
        return right < left;
    }

    RAINY_NODISCARD bool operator>=(thread::id left, thread::id right) noexcept {
        return !(left < right);
    }
#endif
}

namespace rainy::foundation::system::this_thread {
    concurrency::thread::id get_id() noexcept {
        return concurrency::thread::id{concurrency::implements::get_thread_id()};
    }

    void yield() noexcept {
        return concurrency::implements::thread_yield();
    }
}