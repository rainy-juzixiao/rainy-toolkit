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
#include <rainy/linux_api/system/multithread/thread.h>
#if RAINY_USING_LINUX

namespace rainy::linux_api::system::multithread {
    void endthread() {
        pthread_exit(nullptr);
    }

    void endthreadex(unsigned int return_code) {
        pthread_exit(&return_code);
    }

    std::uintptr_t create_thread(foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
                                 unsigned int stack_size, void *arg_list) {
        using arg_type = std::tuple<foundation::functional::function_pointer<unsigned int (*)(void *)>, void *>;
        if (!invoke_function_addr) {
            errno = EINVAL;
            return -1;
        }
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        if (stack_size > 0) {
            if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
                errno = EINVAL; // Invalid stack size
                return -1;
            }
        }
        pthread_t thread{};
        int ret = pthread_create(&thread, &attr, reinterpret_cast<void *(*) (void *)>(invoke_function_addr.get()), arg_list);
        /*
        上层通常要求unsigned int (*)(void *)
            作为函数指针签名。posix的接口中void *返回值很多时候是返回0的，为什么不能提供一个直观的接口呢。而很有意思的是。如果void
                *作为返回值。或许，这个返回值表示的是个intptr_t，所以unsigned int也差不多了。为什么?unsigned int足够表示了。
        */
        if (ret != 0) {
            if (ret == EAGAIN) {
                errno = EAGAIN;
            } else if (ret == EINVAL) {
                errno = EINVAL;
            } else if (ret == EPERM) {
                errno = EACCES;
            }
#if RAINY_ENABLE_DEBUG
            throw std::system_error(errno, std::system_category());
#else
            return -1;
#endif
        }
        pthread_attr_destroy(&attr);
        return reinterpret_cast<std::uintptr_t>(thread);
    }

    RAINY_NODISCARD std::uintptr_t create_thread(
        void *security, unsigned int stack_size,
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
                return -1;
            }
        }
        pthread_t thread{};
        int ret = pthread_create(&thread, &attr, reinterpret_cast<void *(*) (void *)>(invoke_function_addr.get()), arg_list);
        if (ret != 0) {
            if (ret == EAGAIN) {
                errno = EAGAIN;
            } else if (ret == EINVAL) {
                errno = EINVAL;
            } else if (ret == EPERM) {
                errno = EACCES;
            }
#if RAINY_ENABLE_DEBUG
            throw std::system_error(errno, std::system_category());
#else
            return -1;
#endif
        }
        if (thrd_addr) {
            *thrd_addr = thread;
        }
        pthread_attr_destroy(&attr);
        return reinterpret_cast<std::uintptr_t>(thread);
    }

    posix_thread_result thread_join(std::uintptr_t thread_handle, std::int64_t *result_receiver) noexcept {
        if (!thread_handle) {
            errno = EINVAL;
            return posix_thread_result::error;
        }
        void *thread_result = nullptr;
        int join_result = pthread_join(reinterpret_cast<pthread_t>(thread_handle), &thread_result);
        if (result_receiver) {
            *result_receiver = static_cast<int>(reinterpret_cast<std::intptr_t>(thread_result));
        }
        if (join_result == 0) {
            return posix_thread_result::success;
        }
        switch (join_result) {
            case EINTR:
                RAINY_FALLTHROUGH; // [[fallthrough]]
            case EINVAL:
                RAINY_FALLTHROUGH; // [[fallthrough]]
            case ESRCH:
                errno = EINVAL;
                return posix_thread_result::error;
            case EDEADLK:
                return posix_thread_result::busy;
            default:
                return posix_thread_result::error;
        }
    }

    posix_thread_result thread_detach(std::uintptr_t thread_handle) noexcept {
        if (!thread_handle) {
            errno = EINVAL;
            return posix_thread_result::error;
        }
        int detach_result = pthread_detach(reinterpret_cast<pthread_t>(thread_handle));
        if (detach_result == 0) {
            return posix_thread_result::success;
        }
        switch (detach_result) {
            case EINVAL:
                RAINY_FALLTHROUGH; // [[fallthrough]]
            case ESRCH:
                errno = EINVAL;
                return posix_thread_result::error;
            case EPERM:
                errno = EACCES;
                return posix_thread_result::error;
            default:
                return posix_thread_result::error;
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
}

#endif