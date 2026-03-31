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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_BIND_EXECUTOR_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_BIND_EXECUTOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/net/executor/associated_allocator.hpp>
#include <rainy/foundation/io/net/executor/associated_executor.hpp>
#include <rainy/foundation/io/net/executor/async_result.hpp>
#include <rainy/foundation/io/net/executor/executor_trait.hpp>
#include <rainy/foundation/io/net/fwd.hpp>

namespace rainy::foundation::io::net::implements {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_result_type = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_result_type<Ty, type_traits::other_trans::void_t<typename Ty::result_type>> = true;

    template <typename Ty, bool = has_result_type<Ty>>
    struct executor_binder_result_type_impl {
        using result_type = typename Ty::result_type; // NOLINT
        using result_type_or_void = result_type;
    };

    template <typename Ty>
    struct executor_binder_result_type_impl<Ty, false> {
        using result_type_or_void = void;
    };

    template <typename Rx, bool Has>
    struct executor_binder_result_type_impl<Rx (*)(), Has> {
        using result_type = Rx;
        using result_type_or_void = Rx;
    };

    template <typename Rx, bool Has>
    struct executor_binder_result_type_impl<Rx (&)(), Has> {
        using result_type = Rx;
        using result_type_or_void = Rx;
    };

    template <typename Rx, typename Arg1, bool Has>
    struct executor_binder_result_type_impl<Rx (*)(Arg1), Has> {
        using result_type = Rx;
        using result_type_or_void = Rx;
    };

    template <typename Rx, typename Arg1, bool Has>
    struct executor_binder_result_type_impl<Rx (&)(Arg1), Has> {
        using result_type = Rx;
        using result_type_or_void = Rx;
    };

    template <typename Rx, typename Arg1, typename Arg2, bool Has>
    struct executor_binder_result_type_impl<Rx (*)(Arg1, Arg2), Has> {
        using result_type = Rx;
        using result_type_or_void = Rx;
    };

    template <typename Rx, typename Arg1, typename Arg2, bool Has>
    struct executor_binder_result_type_impl<Rx (&)(Arg1, Arg2), Has> {
        using result_type = Rx;
        using result_type_or_void = Rx;
    };

    template <typename Ty>
    using executor_binder_result_type = executor_binder_result_type_impl<Ty>;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_argument_type = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_argument_type<Ty, type_traits::other_trans::void_t<typename Ty::argument_type>> = true;

    template <typename Ty, bool = has_argument_type<Ty>>
    struct executor_binder_argument_type_impl {
        using argument_type = typename Ty::argument_type; // NOLINT
    };

    template <typename Ty>
    struct executor_binder_argument_type_impl<Ty, false> {};

    template <typename Rx, typename Arg1, bool Has>
    struct executor_binder_argument_type_impl<Rx (*)(Arg1), Has> {
        using argument_type = Arg1;
    };

    template <typename Rx, typename Arg1, bool Has>
    struct executor_binder_argument_type_impl<Rx (&)(Arg1), Has> {
        using argument_type = Arg1;
    };

    template <typename Ty>
    using executor_binder_argument_type = executor_binder_argument_type_impl<Ty>;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_first_argument_type = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_first_argument_type<
        Ty, type_traits::other_trans::void_t<typename Ty::first_argument_type, typename Ty::second_argument_type>> = true;

    template <typename Ty, bool = has_first_argument_type<Ty>>
    struct executor_binder_argument_types_impl {
        using first_argument_type = typename Ty::first_argument_type; // NOLINT
        using second_argument_type = typename Ty::second_argument_type; // NOLINT
    };

    template <typename Ty>
    struct executor_binder_argument_types_impl<Ty, false> {};

    template <typename Rx, typename Arg1, typename Arg2, bool Has>
    struct executor_binder_argument_types_impl<Rx (*)(Arg1, Arg2), Has> {
        using first_argument_type = Arg1;
        using second_argument_type = Arg2;
    };

    template <typename Rx, typename Arg1, typename Arg2, bool Has>
    struct executor_binder_argument_types_impl<Rx (&)(Arg1, Arg2), Has> {
        using first_argument_type = Arg1;
        using second_argument_type = Arg2;
    };

    template <typename Ty>
    using executor_binder_argument_types = executor_binder_argument_types_impl<Ty>;
}

namespace rainy::foundation::io::net::implements {
    template <typename Ty, typename Executor, bool UsesExecutor>
    class executor_binder_base;

    template <typename Ty, typename Executor>
    class executor_binder_base<Ty, Executor, true> : protected Executor {
    protected:
        template <typename E, typename U>
        executor_binder_base(E &&e, U &&u) : executor_(utility::move(e)), target_(executor_arg_t(), executor_, utility::move(u)) {
        }

        Executor executor_;
        Ty target_;
    };

    template <typename Ty, typename Executor>
    class executor_binder_base<Ty, Executor, false> {
    protected:
        template <typename E, typename U>
        executor_binder_base(E &&e, U &&u) : executor_(utility::move(e)), target_(utility::move(u)) {
        }

        Executor executor_;
        Ty target_;
    };
}

namespace rainy::foundation::io::net {
    template <typename Ty, typename Executor>
    class executor_binder : public implements::executor_binder_result_type<Ty>,
                            public implements::executor_binder_argument_type<Ty>,
                            public implements::executor_binder_argument_types<Ty>,
                            private implements::executor_binder_base<Ty, Executor, uses_executor_v<Ty, Executor>> { // NOLINT
    public:
        using target_type = Ty;
        using executor_type = Executor;

        using base = implements::executor_binder_base<Ty, Executor, uses_executor_v<Ty, Executor>>;

        template <typename U>
        executor_binder(executor_arg_t, const executor_type &e, U &&u) : base(e, utility::move(u)) {
        }

        executor_binder(const executor_binder &right) : base(right.get_executor(), right.get()) {
        }

        executor_binder(executor_arg_t, const executor_type &e, const executor_binder &right) : base(e, right.get()) {
        }

        template <typename U, typename OtherExecutor>
        executor_binder(const executor_binder<U, OtherExecutor> &right) : base(right.get_executor(), right.get()) { // NOLINT
        }

        template <typename U, typename OtherExecutor>
        executor_binder(executor_arg_t, const executor_type &e, const executor_binder<U, OtherExecutor> &right) :
            base(e, right.get()) {
        }

        executor_binder(executor_binder &&right) : // NOLINT
            base(utility::move(right.get_executor()), utility::move(right.get())) {
        }

        executor_binder(executor_arg_t, const executor_type &e, executor_binder &&right) : base(e, utility::move(right.get())) {
        }

        template <typename U, typename OtherExecutor>
        executor_binder(executor_binder<U, OtherExecutor> &&right) : // NOLINT
            base(utility::move(right.get_executor()), utility::move(right.get())) {
        }

        template <typename U, typename OtherExecutor>
        executor_binder(executor_arg_t, const executor_type &e, executor_binder<U, OtherExecutor> &&right) :
            base(e, utility::move(right.get())) {
        }

        ~executor_binder() = default;

        target_type &get() noexcept {
            return this->target_;
        }

        const target_type &get() const noexcept {
            return this->target_;
        }

        executor_type get_executor() const noexcept {
            return this->executor_;
        }

        template <typename... Args>
        rain_fn operator()(Args &&...args)->type_traits::type_properties::invoke_result_t<target_type, Args...> {
            return this->target_(utility::forward<Args>(args)...);
        }

        template <typename... Args>
        rain_fn operator()(Args &&...args) const->type_traits::type_properties::invoke_result_t<target_type, Args...> {
            return this->target_(utility::forward<Args>(args)...);
        }
    };

    // NOLINTBEGIN

    template <typename Executor, typename Ty, type_traits::other_trans::enable_if_t<is_executor_v<Executor>, int> = 0>
    RAINY_INLINE rain_fn bind_executor(const Executor &ex, Ty &&t)
        -> executor_binder<typename type_traits::other_trans::decay_t<Ty>, Executor> { // NOLINT
        return executor_binder<typename type_traits::other_trans::decay_t<Ty>, Executor>(executor_arg_t(), ex,
                                                                                         utility::forward<Ty>(t));
    }

    template <typename ExecutionContext, typename Ty,
              type_traits::other_trans::enable_if<
                  type_traits::type_relations::is_convertible_v<ExecutionContext &, execution_context &>, int> = 0>
    RAINY_INLINE rain_fn bind_executor(ExecutionContext &ctx, Ty &&t)
        -> executor_binder<typename type_traits::other_trans::decay_t<Ty>, typename ExecutionContext::executor_type> {
        return executor_binder<typename type_traits::other_trans::decay_t<Ty>, typename ExecutionContext::executor_type>(
            executor_arg_t(), ctx.get_executor(), utility::forward<Ty>(t));
    }

    // NOLINTEND
}

namespace rainy::foundation::io::net {
    template <typename Ty, typename Executor>
    struct uses_executor<executor_binder<Ty, Executor>, Executor> : type_traits::helper::true_type {};

    template <typename Ty, typename Executor, typename Signature>
    class async_result<executor_binder<Ty, Executor>, Signature> {
    public:
        using completion_handler_type = executor_binder<typename async_result<Ty, Signature>::completion_handler_type, Executor>;
        using return_type = async_result<Ty, Signature>::return_type;

        explicit async_result(executor_binder<Ty, Executor> &b) : target_(b.get()) {
        }

        return_type get() {
            return target_.get();
        }

    private:
        async_result(const async_result &) = delete;
        async_result &operator=(const async_result &) = delete;

        async_result<Ty, Signature> target_;
    };

    template <typename Ty, typename Executor, typename Allocator>
    struct associated_allocator<executor_binder<Ty, Executor>, Allocator> {
        using type = typename associated_allocator<Ty, Allocator>::type; // NOLINT

        static type get(const executor_binder<Ty, Executor> &b, const Allocator &a = Allocator()) noexcept {
            return associated_allocator<Ty, Allocator>::get(b.get(), a);
        }
    };

    template <typename Ty, typename Executor, typename Executor1>
    struct associated_executor<executor_binder<Ty, Executor>, Executor1> {
        using type = Executor;

        static type get(const executor_binder<Ty, Executor> &b, const Executor1 & = Executor1()) noexcept {
            return b.get_executor();
        }
    };
}

#endif
