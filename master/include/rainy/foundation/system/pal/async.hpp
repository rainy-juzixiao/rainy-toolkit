#ifndef RAINY_FOUNDATION_SYSTEM_PAL_ASYNC_HPP
#define RAINY_FOUNDATION_SYSTEM_PAL_ASYNC_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/comint/model.hpp>

#if RAINY_USING_WINDOWS
#include <rainy/winapi/pal/async.h>
#else

#endif

namespace rainy::foundation::system::pal::async {
    enum class thrd_result : int {
        success,
        nomem,
        timed_out,
        busy,
        error
    };
}

namespace rainy::foundation::system::pal::async {
    /**
     * @brief 结束当前线程，默认返回0
     * @attention 若要设置自定义返回值，请调用endthreadex
     */
    RAINY_HPP_INLINE void endthread();

    /**
     * @brief 通过本地线程接口创建一个线程
     * @param invoke_function_addr 启动开始执行新线程的例程的地址（invoke_function_addr是一个函数指针，返回值要求为unsigned
     * int，形参为void*）
     * @param stack_size 新线程的堆栈大小或直接填为0
     * @param arg_list 参数列表地址
     * @attention 如果开启调试模式编译，std::system_error将会把errno以及对应的信息进行抛出
     * @remarks 另外
     * @remarks 如果invoke_function_addr为空，则errno为EINVAL，并返回-1
     * @remarks 如果线程创建失败，根据情况设置errno并返回-1
     * @remarks 资源不足 -> EAGAIN
     * @remarks 参数错误 -> EINVAL
     * @remarks 拒绝访问 -> EACESS
     * @return 以uintptr_t的形式返回指向线程句柄的指针，如果失败，返回-1（错误通过errno获取详细信息）
     */
    RAINY_NODISCARD RAINY_HPP_INLINE std::uintptr_t begin_thread(
        functional::function_pointer<unsigned int(void *)> invoke_function_addr, unsigned int stack_size, void *arg_list);

    /**
     * @brief 通过本地线程接口创建一个线程
     * @param security 指向 SECURITY_ATTRIBUTES 结构的指针，此结构确定返回的句柄是否由子进程继承。 如果 security 为
     * nullptr，则不能继承句柄
     * @param stack_size 新线程的堆栈大小或直接填为0
     * @param invoke_function_addr 启动开始执行新线程的例程的地址（invoke_function_addr是一个函数指针，返回值要求为unsigned
     * int，形参为void*）
     * @param arg_list 参数列表地址
     * @param init_flag
     * 控制新线程的初始状态的标志。将initflag设置为0以立即运行，或设置为CREATE_SUSPENDED以在挂起状态下创建线程；使用resume()来恢复此线程。
     * 另外，将initflag设置为STACK_SIZE_PARAM_IS_A_RESERVATION标志以将stack_size用作堆栈的初始保留大小（以字节计）；如果未指定此标志，则stack_size将指定提交大小
     * @param thrd_addr 用于接受线程的标识符的指针，如果为空视为不使用
     * @remarks 如果线程创建失败，根据情况设置errno并返回-1
     * @remarks 资源不足 -> EAGAIN
     * @remarks 参数错误 -> EINVAL
     * @remarks 拒绝访问 -> EACESS
     * @return 以uintptr_t的形式返回指向线程句柄的地址，如果失败，返回-1（错误通过errno获取详细信息）
     */
    RAINY_NODISCARD RAINY_HPP_INLINE std::uintptr_t begin_thread(
        void *security, unsigned int stack_size, functional::function_pointer<unsigned int(void *)> invoke_function_addr,
        void *arg_list, unsigned int init_flag, unsigned int *thrd_addr);

    /**
     * @brief 结束当前线程，并附上一个由用户设置的返回代码
     * @param return_code 线程的返回代码
     */
    RAINY_HPP_INLINE void endthreadex(unsigned int return_code);

    /**
     * @brief 用于等待指定的线程完成执行。当线程成功完成时，它将获取线程的退出代码并关闭线程句柄
     * @param thread_handle 指向线程句柄的指针。此句柄必须有效并处于活动状态。
     * @param result_receiver 于接受线程的退出码的指针，如果为空视为不使用
     * @return 返回表示操作结果的枚举值：
     *         thrd_result::success 成功等待线程完成，获取退出代码并关闭句柄
     *         thrd_result::error 发生错误（例如参数无效或获取退出代码失败）
     *         thrd_result::busy 等待期间线程被放弃
     *         thrd_result::timed_out 等待超时（此处设置为无限等待，通常不会超时）
     * @attention 如果thread_handle为空或无效，或存在其他参数错误，函数会将errno设置为EINVAL。
     */
    RAINY_HPP_INLINE thrd_result thread_join(void *thread_handle, int *result_receiver) noexcept;

    /**
     * @brief 使线程句柄失效，以防止其被join
     * @attention 调用此函数后，线程将导致未定义行为，不保证其安全性，且也不保证其不发生UB
     */
    RAINY_HPP_INLINE thrd_result thread_detach(void *thread_handle) noexcept;

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
    RAINY_HPP_INLINE unsigned int get_thread_id() noexcept;

    /**
     * @brief 等待指定的时间（单位：毫秒）
     * @param ms 等待时间（单位：毫秒）
     */
    RAINY_HPP_INLINE void thread_sleep_for(const unsigned long ms) noexcept;

    /**
     * @brief 挂起指定线程
     * @param thread_handle 指向线程句柄的指针。此句柄必须有效并处于有效状态
     * @attention 如果thread_handle为空或无效，或存在其他参数错误，函数会将errno设置为EINVAL
     * @attention 另外，如果thread_handle的访问权限不足，errno并不会被设置为EINVAL，而是设置为EACCES
     */
    RAINY_HPP_INLINE void suspend_thread(void *thread_handle) noexcept;

    /**
     * @brief 恢复指定线程的执行
     * @param thread_handle 指向线程句柄的指针。此句柄必须有效并处于有效状态
     * @attention 如果thread_handle为空或无效，或存在其他参数错误，函数会将errno设置为EINVAL
     * @attention 另外，如果thread_handle的访问权限不足，errno并不会被设置为EINVAL，而是设置为EACCES
     */
    RAINY_HPP_INLINE void resume_thread(void *thread_handle) noexcept;
}

namespace rainy::foundation::system::pal::async {
    struct id {
        id() : id_(0) {
        }

        id(unsigned int id) : id_(id) {
        }

        template <typename Elem, typename Traits>
        friend std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &os, id data) {
            os << data.id_;
            return os;
        }

        unsigned int id_;
    };

    struct native_thread : foundation::comint::the_unknown {
        ~native_thread() override = default;
        /**
         * @brief 直接调用ABI接口begin_thread创建线程
         * @attention 关于底层错误处理的部分，请参见begin_thread的文档。
         * @attention 另外，如果我们创建线程失败，我们会扔出此库的runtime_error异常（DEBUG模式）或调用std::terminate() （Release模式）
         * @param security 指向 SECURITY_ATTRIBUTES 结构的指针，此结构确定返回的句柄是否由子进程继承。 如果 security 为
         * nullptr，则不能继承句柄
         * @param stack_size 新线程的堆栈大小或直接填为0
         * @param invoke_function_addr 启动开始执行新线程的例程的地址（invoke_function_addr是一个函数指针，返回值要求为unsigned
         * int，形参为void*）
         * @param arg_list 要传递到新线程的参数列表或nullptr
         * @param init_flag
         * 控制新线程的初始状态的标志。将initflag设置为0以立即运行，或设置为CREATE_SUSPENDED以在挂起状态下创建线程；使用resume()来恢复此线程。另外，将initflag设置为STACK_SIZE_PARAM_IS_A_RESERVATION标志以将stack_size用作堆栈的初始保留大小（以字节计）；如果未指定此标志，则stack_size将指定提交大小
         */
        virtual void start(void *security, unsigned int stack_size,
                           functional::function_pointer<unsigned int(void *)> invoke_function_addr, void *arg_list,
                           unsigned int init_flag) = 0;
        /**
         * @brief 执行此方法时，主线程将被阻塞，直到线程结束为止
         */
        virtual void join() noexcept = 0;

        /**
         * @brief 执行此方法时，被分离的线程会独立地持续执行。直到退出。即放弃对当前创建线程的所有权。
         */
        virtual void detach() noexcept = 0;

        /**
         * @brief 获取当前线程id
         * @return 如果当前实例具有线程的所有权，则返回对应的线程id，否则返回0
         */
        RAINY_NODISCARD virtual id get_id() const noexcept = 0;

        /**
         * @brief 挂起此实例的线程（如果为空，则不会发生任何事情）
         */
        virtual void suspend() = 0;

        /**
         * @brief 恢复执行此实例的线程
         * @attention 如果当前没有线程所有权（即如果调用join或detach），则不会发生任何事
         */
        virtual void resume() = 0;

        /**
         * @brief 检查当前线程是否可以调用join或detach（或者说，是否持有线程）
         * @return 如果线程句柄持有，则返回true，否则返回false
         */
        RAINY_NODISCARD virtual bool joinable() const noexcept = 0;

        /**
         * @brief 返回当前线程的句柄
         * @return 当前实例的句柄
         */
        virtual void *native_handle() noexcept = 0;

        /**
         * @brief 实现文件必须实现此接口
         */
        static std::shared_ptr<comint::the_unknown> make_instance();
    };

    /**
     * @brief 初始化所有async模块中的ABI接口，并将其注册到comint模型中
     * @remark 将通过comint模型的接口表注入所有关于此模块的接口，之后便能通过comint模型接口访问此模块所有ABI接口
     */
    RAINY_HPP_INLINE void init_async_moudle_abi();

    /**
     * @brief 终止挂载所有async模块的ABI接口，并从comint模型中注销
     * @remark 调用此函数后，所有关于async模块的接口都将不可用，之后便不能通过comint模型接口访问此模块的任何ABI接口
     */
    RAINY_HPP_INLINE void uninit_async_moudle_abi();
}

#endif