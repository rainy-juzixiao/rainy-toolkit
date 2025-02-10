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
    RAINY_HPP_INLINE void endthread(bool force_exit_main) {
        if (information::is_main_thread() && !force_exit_main) {
            return; // 不应当退出主线程
        }
        if (force_exit_main && information::is_main_thread()) {
            std::exit(0);
            return;
        }
        layer::endthread();
    }

    RAINY_HPP_INLINE void endthreadex(unsigned int return_code, bool force_exit_main) {
        if (information::is_main_thread() && !force_exit_main) {
            return; // 不应当退出主线程
        }
        if (force_exit_main && information::is_main_thread()) {
            std::exit(0);
            return;
        }
        layer::endthreadex(return_code);
    }

    RAINY_NODISCARD RAINY_HPP_INLINE std::uintptr_t create_thread(
        functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, unsigned int stack_size, void *arg_list) {
        return layer::create_thread(invoke_function_addr, stack_size, arg_list);
    }

    RAINY_NODISCARD RAINY_HPP_INLINE std::uintptr_t create_thread(
        void *security, unsigned int stack_size, functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
        void *arg_list, unsigned int init_flag,std::uint64_t * thrd_addr) {
        return layer::create_thread(security, stack_size, invoke_function_addr, arg_list, init_flag, thrd_addr);
    }

    RAINY_HPP_INLINE thrd_result thread_join(std::uintptr_t thread_handle, std::int64_t *result_receiver) noexcept {
        return static_cast<thrd_result>(layer::thread_join(thread_handle, result_receiver));
    }

    RAINY_HPP_INLINE thrd_result thread_detach(std::uintptr_t thread_handle) noexcept {
        return static_cast<thrd_result>(layer::thread_detach(thread_handle));
    }

    RAINY_HPP_INLINE void thread_yield() noexcept {
        layer::thread_yield();
    }

    RAINY_HPP_INLINE unsigned int thread_hardware_concurrency() noexcept {
        return layer::thread_hardware_concurrency();
    }

    RAINY_HPP_INLINE std::uint64_t get_thread_id() noexcept {
        return layer::get_thread_id();
    }

    RAINY_HPP_INLINE void thread_sleep_for(const unsigned long ms) noexcept {
        layer::thread_sleep_for(ms);
    }
}
