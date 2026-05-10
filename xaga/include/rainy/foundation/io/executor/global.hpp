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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTYOR_GLOBAL_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTYOR_GLOBAL_HPP
#include <rainy/foundation/concurrency/mutex.hpp>

namespace rainy::foundation::io::implements {
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
