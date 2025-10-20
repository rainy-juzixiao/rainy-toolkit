#ifndef RAINY_ASYNC_CONCEPTS_CORO_HPP
#define RAINY_ASYNC_CONCEPTS_CORO_HPP
#include <rainy/core/core.hpp>

#if RAINY_HAS_CXX20
#include <coroutine>

namespace rainy::type_traits::concepts {
    template <typename Ty>
    concept awaiter = requires(Ty t, std::coroutine_handle<> c) {
        { t.await_ready() } -> same_as<bool>;
        { t.await_suspend(c) } -> in_types<void, bool, std::coroutine_handle<>>;
        { t.await_resume() };
    };

    template <typename Ty>
    concept member_co_await_awaitable = requires(Ty t) {
        { t.operator co_await() } -> awaiter;
    };

    template <typename Ty>
    concept global_co_await_awaitable = requires(Ty t) {
        { operator co_await(t) } -> awaiter;
    };

    template <typename Ty>
    concept awaitable = member_co_await_awaitable<Ty> || global_co_await_awaitable<Ty> || awaiter<Ty>;

    template <typename Ty>
    concept awaiter_void = awaiter<Ty> && requires(Ty t) {
        { t.await_resume() } -> same_as<void>;
    };

    template <typename Ty>
    concept member_co_await_awaitable_void = requires(Ty t) {
        { t.operator co_await() } -> awaiter_void;
    };

    template <typename Ty>
    concept global_co_await_awaitable_void = requires(Ty t) {
        { operator co_await(t) } -> awaiter_void;
    };

    template <typename Ty>
    concept awaitable_void = member_co_await_awaitable_void<Ty> || global_co_await_awaitable_void<Ty> || awaiter_void<Ty>;

    template <awaitable awaitable, typename = void>
    struct awaitable_traits {};

    template <awaitable awaitable>
    static auto get_awaiter(awaitable &&value) {
        if constexpr (member_co_await_awaitable<awaitable>) {
            return utility::forward<awaitable>(value).operator co_await();
        } else if constexpr (global_co_await_awaitable<awaitable>) {
            return operator co_await(utility::forward<awaitable>(value));
        } else if constexpr (awaiter<awaitable>) {
            return utility::forward<awaitable>(value);
        }
    }

    template <awaitable awaitable>
    struct awaitable_traits<awaitable> {
        using awaiter_type = decltype(get_awaiter(utility::declval<awaitable>()));
        using awaiter_return_type = decltype(utility::declval<awaiter_type>().await_resume());
    };
}
#endif

#endif