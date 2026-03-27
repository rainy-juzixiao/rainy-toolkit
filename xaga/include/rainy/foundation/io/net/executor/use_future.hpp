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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_USE_FUTURE_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_USE_FUTURE_HPP
#include <rainy/foundation/concurrency/future.hpp>
#include <rainy/foundation/io/net/executor/associated_executor.hpp>

namespace rainy::foundation::io::net::implements {
    template <typename Function, typename Allocator>
    class packaged_token {
    public:
        packaged_token(Function f, const Allocator &a) : function_(utility::move(f)), allocator_(a) {
        }

        Function function_;
        Allocator allocator_;
    };
}

namespace rainy::foundation::io::net {
    template <typename ProtoAllocator = std::allocator<void>>
    class use_future_t {
    public:
        using allocator_type = ProtoAllocator;

        constexpr use_future_t() = default;

        explicit use_future_t(const allocator_type &allocator) : allocator_(allocator) {
        }

        template <typename OtherAllocator>
        use_future_t<OtherAllocator> rebind(const OtherAllocator &allocator) const {
            return use_future_t<OtherAllocator>(allocator);
        }

        RAINY_NODISCARD allocator_type get_allocator() const {
            return allocator_;
        }

        template <typename Function>
        rain_fn operator()(
            Function &&f) const->implements::packaged_token<type_traits::other_trans::decay_t<Function>, allocator_type> {
            return implements::packaged_token<type_traits::other_trans::decay_t<Function>, allocator_type>(
                utility::forward<Function>(f), get_allocator());
        }

    private:
        struct std_allocator_void {
            constexpr std_allocator_void() {
            }

            operator std::allocator<void>() const { // NOLINT
                return std::allocator<void>();
            }
        };

        using member_alloc_t =
            type_traits::other_trans::conditional_t<type_traits::type_relations::is_same_v<std::allocator<void>, allocator_type>,
                                                    std_allocator_void, allocator_type>;
        member_alloc_t allocator_;
    };

    constexpr auto use_future = use_future_t<>();

    template <typename ProtoAllocator, class Result, class... Args>
    class async_result<use_future_t<ProtoAllocator>, Result(Args...)> {
    public:
        using completion_handler_type = implements::packaged_token<typename concurrency::packaged_task<Result(Args...)>,
                                                                   typename use_future_t<ProtoAllocator>::allocator_type>;

        using return_type = typename concurrency::future<Result>;

        explicit async_result(completion_handler_type &h) {
            packaged_task_ = utility::move(h.function_);
            future_ = packaged_task_.get_future();
        }

        return_type get() {
            return utility::move(future_);
        }

    private:
        concurrency::packaged_task<Result(Args...)> packaged_task_;
        concurrency::future<Result> future_;
    };

    template <class Result, class... Args, class Signature>
    class async_result<concurrency::packaged_task<Result(Args...)>, Signature> {
    public:
        using completion_handler_type = concurrency::packaged_task<Result(Args...)>;
        using return_type = void;

        explicit async_result(completion_handler_type &) {
        }

        return_type get() { // NOLINT
        }
    };
}

#endif
