#ifndef RAINY_IMPL_WINAPI_THREAD_CXX
#define RAINY_IMPL_WINAPI_THREAD_CXX
/*
这是一个编译单元（实现文件）我们通常是希望在部分底层API中隐藏实现在Header-Only中。
我们不建议包含此文件，因为cmake会自动将此文件作为编译单元使用
*/
#include <rainy/winapi/api_core.h>

namespace rainy::foundation::system::async::internals::abi {
    void endthread(void) {
        ExitThread(0);
    }

    RAINY_NODISCARD uintptr_t begin_thread(functional::function_pointer<unsigned int(void *)> invoke_function_addr,
                                              unsigned int stack_size, void *arg_list) {
        if (!invoke_function_addr) {
            errno = EINVAL;
            return -1;
        }
        HANDLE handle = CreateThread(nullptr, stack_size, reinterpret_cast<LPTHREAD_START_ROUTINE>(invoke_function_addr.get()),
                                     arg_list, 0, nullptr);
        if (!handle) {
            DWORD error = GetLastError();
            if (error == ERROR_NOT_ENOUGH_MEMORY) {
                errno = EAGAIN;
            } else if (error == ERROR_INVALID_PARAMETER) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
#if RAINY_ENABLE_DEBUG
            throw std::system_error(errno, std::system_category(), strerror(errno));
#endif
            return -1;
        }
        return reinterpret_cast<uintptr_t>(handle);
    }

    RAINY_NODISCARD uintptr_t begin_thread(void *security, unsigned int stack_size,
                                              functional::function_pointer<unsigned int(void *)> invoke_function_addr, void *arg_list,
                                              unsigned int init_flag, unsigned int *thrd_addr) {
        DWORD thread_id = 0;
        if (!invoke_function_addr) {
            errno = EINVAL;
            return 0;
        }
        HANDLE handle =
            CreateThread(static_cast<LPSECURITY_ATTRIBUTES>(security), stack_size,
                         reinterpret_cast<LPTHREAD_START_ROUTINE>(invoke_function_addr.get()), arg_list, init_flag, &thread_id);
        if (thrd_addr) {
            *thrd_addr = thread_id;
        }
        if (!handle) {
            DWORD error = GetLastError();
            if (error == ERROR_NOT_ENOUGH_MEMORY) {
                errno = EAGAIN;
            } else if (error == ERROR_INVALID_PARAMETER) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
#if RAINY_ENABLE_DEBUG
            throw std::system_error(errno, std::system_category(), strerror(errno));
#endif
            return 0;
        }
        return reinterpret_cast<uintptr_t>(handle);
    }

    void endthreadex(unsigned int return_code) {
        ExitThread(return_code);
    }

    thrd_result thread_join(void *thread_handle, int *result_receiver) noexcept {
        if (!thread_handle) {
            errno = EINVAL;
            return thrd_result::error;
        }
        DWORD wait_result = WaitForSingleObject(thread_handle, INFINITE);
        if (wait_result == WAIT_OBJECT_0) {
            static DWORD placeholder;
            DWORD *param_result_receiver = nullptr;
            if (!result_receiver) {
                param_result_receiver = &placeholder;
            } else {
                param_result_receiver = reinterpret_cast<LPDWORD>(result_receiver);
            }
            if (GetExitCodeThread(thread_handle, param_result_receiver) == 0) {
                return thrd_result::error;
            }
            if (CloseHandle(thread_handle)) {
                return thrd_result::success;
            }
            return thrd_result::error;
        }
        DWORD error = GetLastError();
        if (error == ERROR_INVALID_PARAMETER || error == ERROR_INVALID_HANDLE) {
            errno = EINVAL;
            return thrd_result::error;
        }
        if (wait_result == WAIT_ABANDONED) {
            return thrd_result::busy;
        } else if (wait_result == WAIT_TIMEOUT) {
            return thrd_result::timed_out;
        }
        return thrd_result::error;
    }

    thrd_result thread_detach(void *thread_handle) noexcept {
        if (!thread_handle) {
            errno = EINVAL;
            return thrd_result::error;
        }
        if (CloseHandle(thread_handle)) {
            return thrd_result::success;
        }
        return thrd_result::error;
    }

    void thread_yield() noexcept {
        SwitchToThread();
    }

    unsigned int thread_hardware_concurrency() noexcept {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return static_cast<unsigned int>(sysinfo.dwNumberOfProcessors);
    }

    unsigned int get_thread_id() noexcept {
        return GetCurrentThreadId();
    }

    void thread_sleep_for(const unsigned long ms) noexcept {
        Sleep(ms);
    }

    void suspend_thread(void *thread_handle) noexcept {
        if (!thread_handle) {
            return;
        }
        if (SuspendThread(thread_handle) == static_cast<DWORD>(-1)) {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_PARAMETER || error == ERROR_INVALID_HANDLE) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
        }
    }

    void resume_thread(void *thread_handle) noexcept {
        if (!thread_handle) {
            return;
        }
        if (SuspendThread(thread_handle) == static_cast<DWORD>(-1)) {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_PARAMETER || error == ERROR_INVALID_HANDLE) {
                errno = EINVAL;
            } else if (error == ERROR_ACCESS_DENIED) {
                errno = EACCES;
            }
        }
    }
}

namespace rainy::foundation::system::async::internals {
    class native_thread_winimpl : public native_thread {
    public:
        native_thread_winimpl() : thrd() {
        }

        native_thread_winimpl(native_thread_winimpl &&right) noexcept : thrd({}) {
            thrd.id = utility::exchange(right.thrd.id, 0);
            thrd.thrd_handle = utility::exchange(right.thrd.thrd_handle, nullptr);
        }

        virtual ~native_thread_winimpl() {
            if (joinable()) {
#if RAINY_ENABLE_DEBUG
                foundation::system::exceptions::runtime::throw_runtime_error("You must invoke join() method to close this thread.");
#else
                foundation::system::output::stdout_println("You must invoke join() method to close this thread.");
                std::terminate();
#endif
            }
        }

        void start(void *security, unsigned int stack_size, functional::function_pointer<unsigned int(void *)> invoke_function_addr,
                   void *arg_list, unsigned int init_flag) override {
            void *handle = reinterpret_cast<HANDLE>(
                abi::begin_thread(security, stack_size, invoke_function_addr, arg_list, init_flag, &thrd.id.id_));
            if (!handle) {
#if RAINY_ENABLE_DEBUG
                foundation::system::exceptions::runtime::throw_runtime_error("The system resources is unavailable.");
#else
                foundation::system::output::stdout_println("The syttem resources is unavailable.");
                std::terminate();
#endif
            }
            thrd.thrd_handle = handle;
        }

        void join() noexcept override {
            if (!joinable()) {
#if RAINY_ENABLE_DEBUG
                foundation::system::exceptions::runtime::throw_runtime_error("Need joinable() result is true.");
#else
                foundation::system::exceptions::runtime::throw_runtime_error("Need joinable() result is true.");
                std::terminate();
#endif
            }
            if (abi::thread_join(thrd.thrd_handle, nullptr) != thrd_result::success) {
#if RAINY_ENABLE_DEBUG
                foundation::system::exceptions::runtime::throw_runtime_error("Failed to join the thread.");
#else
                foundation::system::output::stdout_println("Failed to join the thread.");
                std::terminate();
#endif
            }
            thrd = {};
        }

        void detach() noexcept override {
            if (abi::thread_detach(thrd.thrd_handle) != thrd_result::success) {
#if RAINY_ENABLE_DEBUG
                foundation::system::exceptions::runtime::throw_runtime_error("Failed to detach the thread.");
#else
                foundation::system::output::stdout_println("Failed to detach the thread.");
                std::terminate();
#endif
            }
            thrd = {};
        }

        id get_id() const noexcept override {
            return thrd.id;
        }

        void suspend() override {
            abi::suspend_thread(thrd.thrd_handle);
        }

        void resume() override {
            abi::resume_thread(thrd.thrd_handle);
        }

        bool joinable() const noexcept override {
            return thrd.id.id_ != 0;
        }

        void *native_handle() noexcept override {
            return thrd.thrd_handle;
        }

        void copy(the_unknown &ptr) override {
            ptr = std::make_shared<native_thread_winimpl>();
        }

        void move(the_unknown &ptr) noexcept {
            ptr = std::make_shared<native_thread_winimpl>(utility::move(*this));
        }

    private:
        struct thrd_t {
            HANDLE thrd_handle{};
            id id{};
        } thrd;
    };

    comint::the_unknown native_thread::make_instance() {
        return std::make_shared<native_thread_winimpl>();
    }

    void init_async_moudle_abi() {
        comint::add_interface<native_thread>();
    }
}

#endif