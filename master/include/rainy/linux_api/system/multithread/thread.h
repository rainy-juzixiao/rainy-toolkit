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
#ifndef RAINY_LINUX_API_SYSTEM_MULTITHREAD_THREAD_H
#define RAINY_LINUX_API_SYSTEM_MULTITHREAD_THREAD_H
#include <pthread.h>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>

namespace rainy::linux_api::system::multithread {
    enum class posix_thread_result : int {
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
    RAINY_HPP_INLINE void endthread();

    /**
     * @brief 结束当前线程，并附上一个由用户设置的返回代码
     * @param return_code 线程的返回代码
     */
    RAINY_HPP_INLINE void endthreadex(unsigned int return_code);

    /**
    * @brief 通过本地线程接口创建一个线程
    * @param invoke_function_addr 启动开始执行新线程的例程的地址（invoke_function_addr是一个函数指针，返回值要求为unsigned
    * int，形参为void*）
    * @param stack_size 新线程的堆栈大小或直接填为0
    * @param arg_list 参数列表地址
    * @attention 如果开启调试模式编译，std::system_error将会把errno以及对应的信息进行抛出
    * @attention 另外，有一点需要注意。为了适配pal接口，因此，我们会将invoke_function_addr强制进行转换以适配linux的线程接口
    * @attention 生命周期需由开发者进行管理
    * @remarks 关于错误处理：
    * @remarks 如果invoke_function_addr为空，则errno为EINVAL，并返回-1
    * @remarks 如果线程创建失败，根据情况设置errno并返回-1
    * @remarks 资源不足 -> EAGAIN
    * @remarks 参数错误 -> EINVAL
    * @remarks 拒绝访问 -> EACESS
    * @return 以uintptr_t的形式返回指向线程句柄的指针，如果失败，返回-1（错误通过errno获取详细信息）
    */
    std::uintptr_t create_thread(foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
                                 unsigned int stack_size, void *arg_list);

   /**
     * @brief 通过本地线程接口创建一个线程
     * @param security （仅限Windows使用，linux请忽略此参数，此处作为pal占位，使用nullptr即可）指向 SECURITY_ATTRIBUTES
     * 结构的指针，此结构确定返回的句柄是否由子进程继承。 如果 security 为 nullptr，则不能继承句柄
     * @param stack_size 新线程的堆栈大小或直接填为0
     * @param invoke_function_addr 启动开始执行新线程的例程的地址（invoke_function_addr是一个函数指针，返回值要求为unsigned
     * int，形参为void*）
     * @param arg_list 参数列表地址
     * @param init_flag
     * （仅限Windows使用，linux请忽略此参数，此处作为pal占位，使用0或NULL即可）控制新线程的初始状态的标志。将initflag设置为0以立即运行，或设置为CREATE_SUSPENDED以在挂起状态下创建线程；使用resume()来恢复此线程。另外，将initflag设置为STACK_SIZE_PARAM_IS_A_RESERVATION标志以将stack_size用作堆栈的初始保留大小（以字节计）；如果未指定此标志，则stack_size将指定提交大小
     * @param thrd_addr 用于接受线程的标识符的指针，如果为空视为不使用
     * @attention 生命周期需由开发者进行管理
     * @remarks 如果线程创建失败，根据情况设置errno并返回-1
     * @remarks 资源不足 -> EAGAIN
     * @remarks 参数错误 -> EINVAL
     * @remarks 拒绝访问 -> EACESS
     * @return 以uintptr_t的形式返回指向线程句柄的地址，如果失败，返回-1（错误通过errno获取详细信息）
     */
    RAINY_NODISCARD RAINY_HPP_INLINE std::uintptr_t create_thread(
        void *security, unsigned int stack_size,
        foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, void *arg_list,
        unsigned int init_flag, std::uint64_t *thrd_addr);
    /**
     * @brief 用于等待指定的线程完成执行。当线程成功完成时，它将获取线程的退出代码并关闭线程句柄
     * @param thread_handle 指向线程句柄的指针。此句柄必须有效并处于活动状态。
     * @param result_receiver 于接受线程的退出码的指针，如果为空视为不使用
     * @return 返回表示操作结果的枚举值：
     *         success 成功等待线程完成，获取退出代码并关闭句柄
     *         error 发生错误（例如参数无效或获取退出代码失败）
     *         busy 等待期间线程被放弃
     * @attention 如果thread_handle为空或无效，或存在其他参数错误，函数会将errno设置为EINVAL。
     */
    RAINY_HPP_INLINE posix_thread_result thread_join(std::uintptr_t thread_handle, std::int64_t *result_receiver) noexcept;

    /**
     * @brief 使线程句柄失效，以防止其被join
     * @attention 调用此函数后，线程将导致未定义行为，不保证其安全性，且也不保证其不发生UB
     */
    RAINY_HPP_INLINE posix_thread_result thread_detach(std::uintptr_t thread_handle) noexcept;

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

#if !RAINY_USING_SOURCESFILE_BUILD
#include <rainy_cxxfiles/linux_api/system/multithread/thread.cxx>
#endif

#endif