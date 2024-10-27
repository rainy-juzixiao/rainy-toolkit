#ifndef RAINY_WINAPI_CORE_H
#define RAINY_WINAPI_CORE_H
#include <rainy/base.hpp>
#include <rainy/meta_programming/type_traits.hpp>

#define RAINY_DECLARE_CHARSET_TEMPLATE \
template<typename CharType = char, \
          rainy::foundation::type_traits::other_transformations::enable_if_t<\
rainy::winapi::type_traits::is_support_charset_v<CharType>, int> = 0>

namespace rainy::information::winapi {
    constexpr internals::raw_string_view<char> winapi_name("rainy.winapi");
}

#include <memory>
#include <string_view>
#include <typeinfo>

namespace rainy::foundation::system::async::internals {
    enum class thrd_result : int {
        success,
        nomem,
        timed_out,
        busy,
        error
    };
}

/*
多线程内部实现封装
此部分详细实现请参见 rainy/sources/win/impl_thread.cxx 
*/
namespace rainy::foundation::system::async::internals::abi {
    void endthread(void);

    RAINY_NODISCARD uintptr_t begin_thread(functional::function_pointer<unsigned int(void *)> invoke_function_addr,
                                              unsigned int stack_size, void *arg_list);

    RAINY_NODISCARD uintptr_t begin_thread(void *security, unsigned int stack_size,
                                              functional::function_pointer<unsigned int(void *)> invoke_function_addr, void *arg_list,
                                              unsigned int init_flag, unsigned int *thrd_addr);

    void endthreadex(unsigned int return_code);

    thrd_result thread_join(void *thread_handle, int *result_receiver) noexcept;

    thrd_result thread_detach(void *thread_handle) noexcept;

    void thread_yield() noexcept;

    unsigned int thread_hardware_concurrency() noexcept;

    unsigned int get_thread_id() noexcept;

    void thread_sleep_for(const unsigned long ms) noexcept;

    void suspend_thread(void *thread_handle) noexcept;

    void resume_thread(void *thread_handle) noexcept;
}

namespace rainy::foundation::system::async::internals {
    struct id {
        id() : id_(0) {
        }

        id(unsigned int id) : id_(id) {
        }

        template <typename Elem, typename Traits>
        friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits> &os, id data) {
            os << data.id_;
            return os;
        }

        unsigned int id_;
    };

    struct native_thread : foundation::comint::the_unknown_t {
        virtual ~native_thread() = default;
        virtual void start(void *security, unsigned int stack_size,
                           functional::function_pointer<unsigned int(void *)> invoke_function_addr, void *arg_list,
                           unsigned int init_flag) = 0;
        virtual void join() noexcept = 0;
        virtual void detach() noexcept = 0;
        virtual id get_id() const noexcept = 0;
        virtual void suspend() = 0;
        virtual void resume() = 0;
        virtual bool joinable() const noexcept = 0;
        virtual void *native_handle() noexcept = 0;
        static the_unknown make_instance();
    };

    void init_async_moudle_abi();
}

namespace rainy::foundation::system::process::internals::abi {
    
}

#endif
