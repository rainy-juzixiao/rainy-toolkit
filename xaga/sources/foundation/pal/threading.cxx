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

#if RAINY_USING_WINDOWS
#include <rainy/winapi/system/multithread/thread.h>
#elif RAINY_USING_LINUX
#include <rainy/linux_api/system/multithread/thread.h>
#endif

namespace rainy::foundation::pal::threading::layer {
#if RAINY_USING_WINDOWS
    using namespace winapi::system::multithread; // layer -> rainy::winapi::system::mutithread
#elif RAINY_USING_LINUX
    using namespace linux_api::system::multithread; // layer -> rainy::linux_api::system::mutithread
#endif
}

namespace rainy::foundation::pal::threading {
    RAINY_INLINE void endthread(bool force_exit_main) {
        if (core::is_main_thread() && !force_exit_main) {
            return; // 不应当退出主线程
        }
        if (force_exit_main && core::is_main_thread()) {
            std::exit(0);
            return;
        }
        layer::endthread();
    }

    RAINY_INLINE void endthreadex(unsigned int return_code, bool force_exit_main) {
        if (core::is_main_thread() && !force_exit_main) {
            return; // 不应当退出主线程
        }
        if (force_exit_main && core::is_main_thread()) {
            std::exit(0);
            return;
        }
        layer::endthreadex(return_code);
    }

    RAINY_NODISCARD std::uintptr_t create_thread(
        functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, unsigned int stack_size, void *arg_list) {
        return layer::create_thread(invoke_function_addr, stack_size, arg_list);
    }

    RAINY_NODISCARD std::uintptr_t create_thread(
        void *security, unsigned int stack_size, functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
        void *arg_list, unsigned int init_flag,std::uint64_t * thrd_addr) {
        return layer::create_thread(security, stack_size, invoke_function_addr, arg_list, init_flag, thrd_addr);
    }

    RAINY_INLINE thrd_result thread_join(std::uintptr_t thread_handle, std::int64_t *result_receiver) noexcept {
        return static_cast<thrd_result>(layer::thread_join(thread_handle, result_receiver));
    }

    RAINY_INLINE thrd_result thread_detach(std::uintptr_t thread_handle) noexcept {
        return static_cast<thrd_result>(layer::thread_detach(thread_handle));
    }

    RAINY_INLINE void thread_yield() noexcept {
        layer::thread_yield();
    }

    RAINY_INLINE unsigned int thread_hardware_concurrency() noexcept {
        return layer::thread_hardware_concurrency();
    }

    RAINY_INLINE std::uint64_t get_thread_id() noexcept {
        return layer::get_thread_id();
    }

    RAINY_INLINE void thread_sleep_for(const unsigned long ms) noexcept {
        layer::thread_sleep_for(ms);
    }
}

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
                // manual模式表示正常threading模式，如果使用manual模式，由用户自己确保线程生命周期，如果此处一旦出现内存泄漏，必须进行释放。因此，此处不会允许程序的正常执行
                if (joinable()) {
                    std::terminate();
                }
                break;
        }
    }

    thread &thread::operator=(thread &&right) noexcept {
        utility::expects(!joinable(), "We can't deprecated this thread. it's still exists. unless using detach() or join()",
                         utility::contract_option::use_terminate);
        this->thread_handle = utility::exchange(right.thread_handle, 0);
        this->id = utility::exchange(right.id, 0);
        this->policy_ = utility::exchange(right.policy_, policy::manual);
        return *this;
    }

    void thread::join() {
        utility::expects(joinable(), "Current Thread Is Invalid", utility::contract_option::use_exception);
        utility::expects(id != get_thread_id(), "Resource deadlock would occur", utility::contract_option::use_exception);
        auto result = thread_join(thread_handle, nullptr);
        if (result == thrd_result::success) {
            thread_handle = 0;
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
        utility::expects(id != get_thread_id(), "Resource deadlock would occur", utility::contract_option::use_exception);
        utility::ensures(thread_detach(thread_handle) == thrd_result::success, "Failed to detach current thread.",
                         utility::contract_option::use_exception);
        thread_handle = 0;
    }

    bool thread::joinable() const noexcept {
        return static_cast<bool>(thread_handle);
    }

    void thread::sleep_for(const unsigned long ms) noexcept {
        thread_sleep_for(ms);
    }

    RAINY_NODISCARD thread::native_handle_type thread::native_handle() const noexcept {
        return reinterpret_cast<native_handle_type>(thread_handle);
    }

    RAINY_NODISCARD std::uintptr_t thread::get() const noexcept {
        return thread_handle;
    }
}
