#ifndef RAINY_FOUNDATION_IO_NET_EXECUTYOR_GLOBAL_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTYOR_GLOBAL_HPP
#include <rainy/foundation/concurrency/mutex.hpp>

namespace rainy::foundation::io::net::implements {
    template <typename Ty>
    struct global_impl {
        static void do_init() {
            instance_.ptr_ = new Ty;
        }

        ~global_impl() {
            delete ptr_;
        }

        static concurrency::once_flag init_once_;
        static global_impl instance_;
        Ty *ptr_;
    };

    template <typename Ty>
    concurrency::once_flag global_impl<Ty>::init_once_;

    template <typename Ty>
    global_impl<Ty> global_impl<Ty>::instance_;

    template <typename Ty>
    Ty &global() {
        concurrency::call_once(global_impl<Ty>::init_once_, &global_impl<Ty>::do_init);
        return *global_impl<Ty>::instance_.ptr_;
    }
}

#endif
