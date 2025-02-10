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
#ifndef RAINY_FOUNDATION_PAL_THREADING_HPP
#define RAINY_FOUNDATION_PAL_THREADING_HPP

#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/system/memory/nebula_ptr.hpp>

#if RAINY_USING_LINUX
#include <pthread.h>
#endif

namespace rainy::foundation::pal::threading {
    enum class thrd_result : int {
        success,
        nomem,
        timed_out,
        busy,
        error
    };

    /**
     * @brief 结束当前线程，默认返回0
     * @attention 若要设置自定义返回值，请调用endthreadex
     */
    RAINY_HPP_INLINE void endthread(bool force_exit_main = false);

    /**
     * @brief 结束当前线程，并附上一个由用户设置的返回代码
     * @param return_code 线程的返回代码
     */
    RAINY_HPP_INLINE void endthreadex(unsigned int return_code, bool force_exit_main = false);

    /**
     * @brief 通过本地线程接口创建一个线程
     * @param invoke_function_addr 启动开始执行新线程的例程的地址（invoke_function_addr是一个函数指针，返回值要求为unsigned
     * int，形参为void*）
     * @param stack_size 新线程的堆栈大小或直接填为0
     * @param arg_list 参数列表地址
     * @attention 如果开启调试模式编译，std::system_error将会把errno以及对应的信息进行抛出
     * @attention 生命周期需由开发者进行管理
     * @remarks 另外
     * @remarks 如果invoke_function_addr为空，则errno为EINVAL，并返回-1
     * @remarks 如果线程创建失败，根据情况设置errno并返回-1
     * @remarks 资源不足 -> EAGAIN
     * @remarks 参数错误 -> EINVAL
     * @remarks 拒绝访问 -> EACESS
     * @return 以intptr_t的形式返回指向线程句柄的指针，如果失败，返回-1（错误通过errno获取详细信息）
     */
    RAINY_NODISCARD RAINY_HPP_INLINE std::uintptr_t create_thread(
        functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, unsigned int stack_size, void *arg_list);

    /**
     * @brief 通过本地线程接口创建一个线程
     * @param security （仅限Windows使用，linux请忽略此参数）指向 SECURITY_ATTRIBUTES 结构的指针，此结构确定返回的句柄是否由子进程继承。 如果 security 为
     * nullptr，则不能继承句柄
     * @param stack_size 新线程的堆栈大小或直接填为0
     * @param invoke_function_addr 启动开始执行新线程的例程的地址（invoke_function_addr是一个函数指针，返回值要求为unsigned
     * int，形参为void*）
     * @param arg_list 参数列表地址
     * @param init_flag （仅限Windows使用，linux请忽略此参数）控制新线程的初始状态的标志。将initflag设置为0以立即运行，或设置为CREATE_SUSPENDED以在挂起状态下创建线程；使用resume()来恢复此线程。另外，将initflag设置为STACK_SIZE_PARAM_IS_A_RESERVATION标志以将stack_size用作堆栈的初始保留大小（以字节计）；如果未指定此标志，则stack_size将指定提交大小
     * @param thrd_addr 用于接受线程的标识符的指针，如果为空视为不使用
     * @attention 生命周期需由开发者进行管理
     * @remarks 如果线程创建失败，根据情况设置errno并返回-1
     * @remarks 资源不足 -> EAGAIN
     * @remarks 参数错误 -> EINVAL
     * @remarks 拒绝访问 -> EACESS
     * @return 以uintptr_t的形式返回指向线程句柄的地址，如果失败，返回-1（错误通过errno获取详细信息）
     */
    RAINY_NODISCARD RAINY_HPP_INLINE std::uintptr_t create_thread(
        void *security, unsigned int stack_size, functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
        void *arg_list, unsigned int init_flag, std::uint64_t *thrd_addr);

    /**
     * @brief 用于等待指定的线程完成执行。当线程成功完成时，它将获取线程的退出代码并关闭线程句柄
     * @param thread_handle 指向线程句柄的指针。此句柄必须有效并处于活动状态。
     * @param result_receiver 于接受线程的退出码的指针，如果为空视为不使用
     * @return 返回表示操作结果的枚举值：
     *         success 成功等待线程完成，获取退出代码并关闭句柄
     *         error 发生错误（例如参数无效或获取退出代码失败）
     *         busy 等待期间线程被放弃
     *         timed_out （Windows将会返回此值，linux不会）等待超时（此处设置为无限等待，通常不会超时）
     * @attention 如果thread_handle为空或无效，或存在其他参数错误，函数会将errno设置为EINVAL。
     */
    RAINY_HPP_INLINE thrd_result thread_join(std::uintptr_t thread_handle, std::int64_t *result_receiver) noexcept;

    /**
     * @brief 使线程句柄失效，以防止其被join
     * @attention 调用此函数后，线程将导致未定义行为，不保证其安全性，且也不保证其不发生UB
     */
    RAINY_HPP_INLINE thrd_result thread_detach(std::uintptr_t thread_handle) noexcept;

    /**
     * @brief 让出当前线程的执行权
     */
    RAINY_HPP_INLINE void thread_yield() noexcept;

    /**
     * @brief 获取当前线程的硬件线程数
     * @return 成功获取线程数，返回大于等于1的整数；失败（例如系统调用失败），返回0
     */
    RAINY_HPP_INLINE unsigned int thread_hardware_concurrency() noexcept;

    /**
     * @brief 获取当前线程的id
     * @return 成功获取线程id，返回大于等于1的整数；失败（例如系统调用失败），返回0
     */
    RAINY_HPP_INLINE std::uint64_t get_thread_id() noexcept;

    /**
     * @brief 等待指定的时间（单位：毫秒）
     * @param ms 等待时间（单位：毫秒）
     */
    RAINY_HPP_INLINE void thread_sleep_for(const unsigned long ms) noexcept;
}

namespace rainy::foundation::pal::threading {
    class thread {
    public:
#if RAINY_USING_LINUX
        using native_handle_type = ::pthread_t;
#else
        using native_handle_type = void*;
#endif

        thread() : id(0), thread_handle(0) {
        }

        thread(thread &&right) noexcept :
            id(utility::exchange(right.id, 0)), thread_handle(utility::exchange(right.thread_handle, 0)) {
        }

        template <typename Fx, typename... Args>
        explicit thread(Fx &&f, Args &&...args) {
            start(utility::forward<Fx>(f), utility::forward<Args>(args)...);
        }

        ~thread() {
            if (joinable()) {
                std::terminate();
            }
        }

        thread &operator=(thread &&right) noexcept {
            utility::expects(!joinable(), "We can't deprecated this thread. it's still exists. unless using detach() or join()",
                             utility::contract_option::use_terminate);
            this->thread_handle = utility::exchange(right.thread_handle, 0);
            this->id = utility::exchange(right.id, 0);
            return *this;
        }

        thread(const thread &) = delete;
        thread &operator=(const thread &) = delete;

        template <typename Fx, typename... Args>
        void start(Fx &&fn, Args &&...args) {
            using tuple = std::tuple<type_traits::other_trans::decay_t<Fx>, type_traits::other_trans::decay_t<Args>...>;
            if (thread_handle) {
                return;
            }
            auto decay_copied =
                foundation::system::memory::make_nebula<tuple>(utility::forward<Fx>(fn), utility::forward<Args>(args)...);
            constexpr auto invoker = get_invoke_function_addr<tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
            thread_handle = create_thread(nullptr, 0, invoker, decay_copied.get(), 0, &id);
            if (thread_handle) {
                (void) decay_copied.release();
            } else {
                throw std::system_error(errno, std::generic_category(), "Failed to create thread");
            }
        }

        void join() {
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
                    utility::ensures(
                        false,
                        "Failed to join thread. because [thread_join() - pal] has found a unknown error consider debug this "
                        "function [thread_join() - pal]",
                        utility::contract_option::use_exception);
                    break;
            }
        }

        void detach() {
            utility::expects(joinable(), "Current Thread Is Invalid", utility::contract_option::use_exception);
            utility::expects(id != get_thread_id(), "Resource deadlock would occur", utility::contract_option::use_exception);
            utility::ensures(thread_detach(thread_handle) == thrd_result::success, "Failed to detach current thread.",
                             utility::contract_option::use_exception);
            thread_handle = 0;
        }

        bool joinable() const noexcept {
            return static_cast<bool>(thread_handle);
        }

        void sleep_for(const unsigned long ms) const noexcept {
            thread_sleep_for(ms);
        }

        native_handle_type native_handle() noexcept {
            return reinterpret_cast<native_handle_type>(thread_handle);
        }

        std::uintptr_t get() noexcept {
            return thread_handle;
        }

    private:
        template <typename Tuple, std::size_t... Indices>
        static unsigned int invoke_function(void *arg_list) {
            foundation::system::memory::nebula_ptr<Tuple> fn_vals(static_cast<Tuple *>(arg_list));
            Tuple &tuple = *fn_vals.get();
            utility::invoke(utility::move(std::get<Indices>(tuple))...);
            return 0;
        }

        template <typename Tuple, std::size_t... Indices>
        static constexpr functional::function_pointer<unsigned int (*)(void *)> get_invoke_function_addr(
            std::index_sequence<Indices...>) {
            return &invoke_function<Tuple, Indices...>;
        }

        std::uint64_t id;
        std::uintptr_t thread_handle = 0;
    };

    template <typename Fx, typename... Args>
    thread make_thread(Fx &&fn, Args &&...args) {
        return thread(utility::forward<Fx>(fn), utility::forward<Args>(args)...);
    }
}

#if !RAINY_USING_SOURCESFILE_BUILD
#include <rainy_cxxfiles/foundation/pal/threading.cxx>
#endif

#endif