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
#include <unistd.h>

#if RAINY_USING_MACOS
#include <sys/sysctl.h> // NOLINT
#else
#include <sys/syscall.h> // NOLINT
#endif

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
#if RAINY_USING_MACOS
        std::uint64_t tid{0};
        pthread_t pthread_handle{};
#else
        pid_t tid{0};
#endif
        functional::function_pointer<unsigned int (*)(void *)> invoke_address{};
        void *arg_list{nullptr};
        volatile long tid_ready{0};
    };

    unsigned int invoke_thread_task(void *task_structure) {
        install_signal_handlers();
        rainy_let task = static_cast<the_task *>(task_structure);
        auto *const arg_list = task->arg_list;
        const auto execute_task = task->invoke_address;
        {
#if RAINY_USING_MACOS
            std::uint64_t tid64 = 0;
            pthread_threadid_np(nullptr, &tid64);
            task->tid = tid64;
            task->pthread_handle = pthread_self();
#else
            task->tid = static_cast<pid_t>(syscall(SYS_gettid));
#endif
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
                errno = EINVAL;
                return {};
            }
        }
        // NOLINTBEGIN
        the_task task{
#if RAINY_USING_MACOS
            0,
            {},
#else
            0,
#endif
            invoke_function_addr,
            arg_list,
            false};
        pthread_t thread{};
        // NOLINTEND
        const int ret = pthread_create(&thread, &attr, reinterpret_cast<void *(*) (void *)>(&invoke_thread_task), &task);
        pthread_attr_destroy(&attr);
        if (ret != 0) {
            errno = ret;
            return {};
        }
        while (core::pal::iso_volatile_load(&task.tid_ready) == 0) {
            thread_yield();
        }
#if RAINY_USING_MACOS
        return {reinterpret_cast<std::uintptr_t>(thread), static_cast<std::uint64_t>(task.tid)};
#else
        return {thread, static_cast<std::uint64_t>(task.tid)}; // NOLINT
#endif
    }

    schd_thread_t create_thread(void *security, unsigned int stack_size,
                                foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
                                void *arg_list, unsigned int init_flag, std::uint64_t *thrd_addr) {
        (void) init_flag;
        (void) security;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        if (stack_size > 0) {
            if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
                pthread_attr_destroy(&attr);
                errno = EINVAL;
                return {};
            }
        }
        pthread_t thread{};
        // NOLINTBEGIN
        the_task task{
#if RAINY_USING_MACOS
            0,
            {},
#else
            0,
#endif
            invoke_function_addr,
            arg_list,
            false};
        // NOLINTEND
        const int ret = pthread_create(&thread, &attr, reinterpret_cast<void *(*) (void *)>(&invoke_thread_task), &task);
        pthread_attr_destroy(&attr);
        if (ret != 0) {
            errno = ret;
            return {};
        }
        if (thrd_addr) {
#if RAINY_USING_MACOS
            *thrd_addr = reinterpret_cast<std::uint64_t>(thread);
#else
            *thrd_addr = thread;
#endif
        }
        while (core::pal::iso_volatile_load(&task.tid_ready) == 0) {
        }
        return {reinterpret_cast<std::uintptr_t>(thread), static_cast<std::uint64_t>(task.tid)}; // NOLINT
    }

    thrd_result thread_join(schd_thread_t thread_handle, std::int64_t *result_receiver) noexcept {
        if (!thread_handle.handle) {
            errno = EINVAL;
            return thrd_result::error;
        }
        void *thread_result = nullptr;
        const int join_result = pthread_join(reinterpret_cast<pthread_t>(thread_handle.handle), &thread_result); // NOLINT
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
#if RAINY_USING_MACOS
        const int detach_result = pthread_detach(reinterpret_cast<pthread_t>(thread_handle.handle));
#else
        const int detach_result = pthread_detach(thread_handle.handle);
#endif
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
#if RAINY_USING_MACOS
        // macOS 的 pthread_getconcurrency() 永远返回 0，改用 sysctl 查询逻辑核心数
        int count = 0;
        std::size_t size = sizeof(count);
#if RAINY_USING_MACOS_AND_IS_APPLE_SILICON
        // Apple Silicon 使用 hw.perflevel0.logicalcpu 查询高性能核心数，
        // 再加上 hw.perflevel1.logicalcpu 查询能效核心数，合计即为全部逻辑核心数
        int perf_cores = 0, eff_cores = 0;
        std::size_t perf_size = sizeof(perf_cores), eff_size = sizeof(eff_cores);
        if (sysctlbyname("hw.perflevel0.logicalcpu", &perf_cores, &perf_size, nullptr, 0) == 0 &&
            sysctlbyname("hw.perflevel1.logicalcpu", &eff_cores, &eff_size, nullptr, 0) == 0) {
            return static_cast<unsigned int>(perf_cores + eff_cores);
        }
#endif
        if (sysctlbyname("hw.logicalcpu", &count, &size, nullptr, 0) == 0) {
            return static_cast<unsigned int>(count);
        }
        return 1u; // 最坏情况兜底
#else
        return static_cast<unsigned int>(pthread_getconcurrency());
#endif
    }

    std::uint64_t get_thread_id() noexcept {
#if RAINY_USING_MACOS
        std::uint64_t tid64 = 0;
        pthread_threadid_np(nullptr, &tid64);
        return tid64;
#else
        return static_cast<std::uint64_t>(pthread_self()); // NOLINT
#endif
    }

    void thread_sleep_for(const unsigned long ms) noexcept {
        const auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(ms));
        // NOLINTBEGIN
        timespec req{static_cast<time_t>(total_ns.count() / 1'000'000'000LL), static_cast<long>(total_ns.count() % 1'000'000'000LL)};
        // NOLINTEND
        while (::nanosleep(&req, &req) == -1 && errno == EINTR) {
        }
    }

    void suspend_thread(schd_thread_t thread_handle) noexcept {
        if (!thread_handle.handle) {
            return;
        }
#if RAINY_USING_MACOS
        // macOS 没有 tgkill，通过 pthread_kill 向对应线程投递 SIGUSR1
        // thread_handle.handle 在 macOS 路径下存的是 pthread_t
        pthread_kill(reinterpret_cast<pthread_t>(thread_handle.handle), SIGUSR1);
#else
        (void) syscall(SYS_tgkill, getpid(), thread_handle.tid, SIGUSR1);
#endif
    }

    void resume_thread(schd_thread_t thread_handle) noexcept {
        if (!thread_handle.handle) {
            return;
        }
#if RAINY_USING_MACOS
        pthread_kill(reinterpret_cast<pthread_t>(thread_handle.handle), SIGUSR2);
#else
        (void) syscall(SYS_tgkill, getpid(), thread_handle.tid, SIGUSR2);
#endif
    }
}
