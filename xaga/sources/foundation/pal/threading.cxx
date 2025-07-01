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
#include <rainy/foundation/pal/threading.hpp>

namespace rainy::foundation::pal::threading {
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
        utility::expects(!joinable(), "We can't deprecated this thread. it's still exists. unless using detach() or join()",
                         utility::contract_option::use_terminate);
        this->thread_handle = utility::exchange(right.thread_handle, {});
        this->policy_ = utility::exchange(right.policy_, policy::manual);
        return *this;
    }

    void thread::join() {
        utility::expects(joinable(), "Current Thread Is Invalid", utility::contract_option::use_exception);
        utility::expects(get_id() != thread::id{implements::get_thread_id()}, "Resource deadlock would occur",
                         utility::contract_option::use_exception);
        auto result = implements::thread_join(thread_handle, nullptr);
        if (result == thrd_result::success) {
            thread_handle = {};
            return;
        }
        switch (result) {
            case thrd_result::busy:
                utility::ensures(false, "Failed to join thread. because [thread_join() - pal] returns busy",
                                 utility::contract_option::use_exception);
                break;
            case thrd_result::error:
                utility::ensures(false,
                                 "Failed to join thread. because [thread_join() - pal] returns error. consider debug this "
                                 "function [thread_join() - pal]",
                                 utility::contract_option::use_exception);
                break;
            default:
                utility::ensures(false,
                                 "Failed to join thread. because [thread_join() - pal] has found a unknown error consider debug this "
                                 "function [thread_join() - pal]",
                                 utility::contract_option::use_exception);
                break;
        }
    }

    void thread::detach() {
        utility::expects(joinable(), "Current Thread Is Invalid", utility::contract_option::use_exception);
        utility::expects(get_id() != thread::id{implements::get_thread_id()}, "Resource deadlock would occur",
                         utility::contract_option::use_exception);
        utility::ensures(implements::thread_detach(thread_handle) == thrd_result::success, "Failed to detach current thread.",
                         utility::contract_option::use_exception);
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
}

namespace rainy::foundation::system::this_thread {
    foundation::pal::threading::thread::id get_id() noexcept {
        using namespace foundation::pal;
        return threading::thread::id{threading::implements::get_thread_id()};
    }

    void yield() noexcept {
        return foundation::pal::threading::implements::thread_yield();
    }
}