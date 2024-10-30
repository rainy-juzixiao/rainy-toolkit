#include <coroutine>
#include <rainy/meta/type_traits.hpp>

namespace rainy::foundation::type_traits::concepts {
    template <typename Ty, typename... Types>
    concept in_types = (type_traits::type_relations::is_any_of_v<Ty, Types...>);

    /**
     * This concept declares a type that is required to meet the c++20 coroutine operator co_await()
     * retun type.  It requires the following three member functions:
     *      await_ready() -> bool
     *      await_suspend(std::coroutine_handle<>) -> void|bool|std::coroutine_handle<>
     *      await_resume() -> decltype(auto)
     *          Where the return type on await_resume is the requested return of the awaitable.
     */
    // clang-format off
    template<typename Ty>
    concept awaiter = requires(Ty t, std::coroutine_handle<> c)
    {
        { t.await_ready() } -> std::same_as<bool>;
        { t.await_suspend(c) } -> in_types<void, bool, std::coroutine_handle<>>;
        { t.await_resume() };
    };

    template <typename Ty>
    concept member_co_await_awaitable = requires(Ty t)
    {
        { t.operator co_await() } -> awaiter;
    };

    template <typename Ty>
    concept global_co_await_awaitable = requires(Ty t)
    {
        { operator co_await(t) } -> awaiter;
    };

    /**
     * This concept declares a type that can be operator co_await()'ed and returns an awaiter_type.
     */
    template<typename Ty>
    concept awaitable = member_co_await_awaitable<Ty> || global_co_await_awaitable<Ty> || awaiter<Ty>;

    template<typename Ty>
    concept awaiter_void = awaiter<Ty> && requires(Ty t)
    {
        {t.await_resume()} -> std::same_as<void>;
    };

    template <typename Ty>
    concept member_co_await_awaitable_void = requires(Ty t)
    {
        { t.operator co_await() } -> awaiter_void;
    };

    template <typename Ty>
    concept global_co_await_awaitable_void = requires(Ty t)
    {
        { operator co_await(t) } -> awaiter_void;
    };

    template<typename Ty>
    concept awaitable_void = member_co_await_awaitable_void<Ty> || global_co_await_awaitable_void<Ty> || awaiter_void<Ty>;

    template<awaitable awaitable, typename = void>
    struct awaitable_traits
    {
    };

    template<awaitable awaitable>
    static auto get_awaiter(awaitable&& value)
    {
        if constexpr (member_co_await_awaitable<awaitable>) {
            return utility::forward<awaitable>(value).operator co_await();
        }
        else if constexpr (global_co_await_awaitable<awaitable>) {
            return operator co_await(utility::forward<awaitable>(value));
        }
        else if constexpr (awaiter<awaitable>) {
            return utility::forward<awaitable>(value);
        }
    }

    template<awaitable awaitable>
    struct awaitable_traits<awaitable>
    {
        using awaiter_type        = decltype(get_awaiter(std::declval<awaitable>()));
        using awaiter_return_type = decltype(std::declval<awaiter_type>().await_resume());
    };
}