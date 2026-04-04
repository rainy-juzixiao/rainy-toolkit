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
#include <csignal>
#include <pthread.h>
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/pal.hpp>

namespace rainy::foundation::concurrency::implements {
    void endthread() {
        pthread_exit(nullptr);
    }

    void endthreadex(unsigned int return_code) {
        pthread_exit(&return_code);
    }

    thread_local volatile sig_atomic_t paused = 0;

    void sigusr1_handler(int sig) {
        paused = 1;
        while (paused) {
            pause();
        }
    }

    void sigusr2_handler(int sig) {
        paused = 0;
    }

    void install_signal_handlers() {
        struct sigaction sa1{};
        sa1.sa_handler = sigusr1_handler;
        sigemptyset(&sa1.sa_mask);
        sa1.sa_flags = SA_RESTART;
        sigaction(SIGUSR1, &sa1, nullptr);
        struct sigaction sa2{};
        sa2.sa_handler = sigusr2_handler;
        sigemptyset(&sa2.sa_mask);
        sa2.sa_flags = SA_RESTART;
        sigaction(SIGUSR2, &sa2, nullptr);
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigaddset(&mask, SIGUSR2);
        pthread_sigmask(SIG_UNBLOCK, &mask, nullptr);
    }

    struct the_task {
        pid_t tid{0};
        functional::function_pointer<unsigned int (*)(void *)> invoke_address{};
        void *arg_list{nullptr};
        volatile long tid_ready{0};
    };

    unsigned int invoke_thread_task(void *task_structure) {
        install_signal_handlers();
        auto tid = static_cast<pid_t>(syscall(SYS_gettid));
        auto *task = reinterpret_cast<the_task *>(task_structure);
        auto arg_list = task->arg_list;
        auto execute_task = task->invoke_address;
        {
            task->tid = tid;
            core::pal::interlocked_exchange(&task->tid_ready, 0xA0);
        }
        return execute_task(arg_list);
    }

    schd_thread_t create_thread(foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
                                unsigned int stack_size, void *arg_list) {
        if (!invoke_function_addr) {
            errno = EINVAL;
            return {};
        }
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        if (stack_size > 0) {
            if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
                errno = EINVAL; // Invalid stack size
                return {};
            }
        }
        the_task task{
            0, invoke_function_addr, arg_list,
            false}; // task记录了创建的具体任务信息的上下文（因为以值信息存在且会持续等待直到拿到tid，因此在这一点上，生命周期问题是无的）
        pthread_t thread{};
        const int ret = pthread_create(&thread, &attr, reinterpret_cast<void *(*) (void *)>(&invoke_thread_task), &task);
        /*
        上层通常要求unsigned int (*)(void *)
            作为函数指针签名。posix的接口中void *返回值很多时候是返回0的，为什么不能提供一个直观的接口呢。而很有意思的是。如果void
                *作为返回值。或许，这个返回值表示的是个intptr_t，所以unsigned int也差不多了。为什么?unsigned int足够表示了。
        */
        pthread_attr_destroy(&attr);
        if (ret != 0) {
            errno = ret;
            return {};
        }
        while (core::pal::iso_volatile_load(&task.tid_ready) == 0) { // 这里将会持续轮询，同时在等待的时候让出执行权，直到拿到tid
            thread_yield();
        }
        return {thread, static_cast<std::uint64_t>(task.tid)};
    }

    RAINY_NODISCARD schd_thread_t
    create_thread(void *security, unsigned int stack_size,
                  foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, void *arg_list,
                  unsigned int init_flag, std::uint64_t *thrd_addr) {
        (void) init_flag; // We don't need to set initial state. Just deprecated.
        (void) security;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        if (stack_size > 0) {
            if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
                pthread_attr_destroy(&attr);
                errno = EINVAL; // Invalid stack size
                return {};
            }
        }
        pthread_t thread{};
        the_task task{0, invoke_function_addr, arg_list, false}; // 分配the_task对象
        const int ret = pthread_create(&thread, &attr, reinterpret_cast<void *(*) (void *)>(&invoke_thread_task), &task);
        pthread_attr_destroy(&attr);
        if (ret != 0) {
            errno = ret;
            return {};
        }
        if (thrd_addr) {
            *thrd_addr = thread;
        }
        while (core::pal::iso_volatile_load(&task.tid_ready) == 0) {
        }
        return {reinterpret_cast<std::uintptr_t>(thread), static_cast<std::uint64_t>(task.tid)};
    }

    thrd_result thread_join(schd_thread_t thread_handle, std::int64_t *result_receiver) noexcept {
        if (!thread_handle.handle) {
            errno = EINVAL;
            return thrd_result::error;
        }
        void *thread_result = nullptr;
        const int join_result = pthread_join(reinterpret_cast<pthread_t>(thread_handle.handle), &thread_result);
        if (result_receiver) {
            *result_receiver = static_cast<int>(reinterpret_cast<std::intptr_t>(thread_result));
        }
        if (join_result == 0) {
            return thrd_result::success;
        }
        switch (join_result) {
            case EINTR:
            case EINVAL:
            case ESRCH:
                errno = EINVAL;
                return thrd_result::error;
            case EDEADLK:
                return thrd_result::busy;
            default:
                return thrd_result::error;
        }
    }

    thrd_result thread_detach(schd_thread_t thread_handle) noexcept {
        if (!thread_handle.handle) {
            errno = EINVAL;
            return thrd_result::error;
        }
        const int detach_result = pthread_detach(thread_handle.handle);
        if (detach_result == 0) {
            return thrd_result::success;
        }
        switch (detach_result) {
            case EINVAL:
            case ESRCH:
                errno = EINVAL;
                return thrd_result::error;
            case EPERM:
                errno = EACCES;
            default:
                return thrd_result::error;
        }
    }

    void thread_yield() noexcept {
        sched_yield();
    }

    unsigned int thread_hardware_concurrency() noexcept {
        return static_cast<unsigned int>(pthread_getconcurrency());
    }

    std::uint64_t get_thread_id() noexcept {
        return static_cast<std::uint64_t>(pthread_self());
    }

    void thread_sleep_for(const unsigned long ms) noexcept {
        (void) ms;
        timespec req{0, std::chrono::milliseconds(ms).count()};
        while (::nanosleep(&req, &req) == -1 && errno == EINTR) {
        }
    }

    void suspend_thread(schd_thread_t thread_handle) noexcept {
        if (thread_handle.handle) {
            (void) syscall(SYS_tgkill, getpid(), thread_handle.tid, SIGUSR1);
        }
    }

    void resume_thread(schd_thread_t thread_handle) noexcept {
        if (thread_handle.handle) {
            (void) syscall(SYS_tgkill, getpid(), thread_handle.tid, SIGUSR2);
        }
    }
}
