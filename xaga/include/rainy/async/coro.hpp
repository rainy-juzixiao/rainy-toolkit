#ifndef RAINY_ASYNC_CORO_HPP
#define RAINY_ASYNC_CORO_HPP
#include <rainy/core/core.hpp>

#if RAINY_HAS_CXX20
#include <rainy/async/concepts/coro.hpp>
#include <condition_variable>
#include <coroutine>
#include <stdexcept>
#include <utility>
#include <variant>

namespace rainy::async {
    template <typename return_type = void>
    class coroutine_task;

    template <typename return_type>
    struct promise;
}

namespace rainy::async::implements {
    struct promise_base {
        friend struct final_awaitable;
        struct final_awaitable {
            static bool await_ready() noexcept {
                return false;
            }

            template <typename promise_type>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> coroutine) noexcept {
                if (auto &promise = coroutine.promise(); promise.handler != nullptr) {
                    return promise.handler;
                }
                return std::noop_coroutine();
            }

            static void await_resume() noexcept {
            }
        };

        promise_base() noexcept = default;
        ~promise_base() = default;

        static std::suspend_always initial_suspend() noexcept {
            return {};
        }

        static final_awaitable final_suspend() noexcept {
            return {};
        }

        void continuation(const std::coroutine_handle<> continuation) noexcept {
            handler = continuation;
        }

    protected:
        std::coroutine_handle<> handler{nullptr};
    };
}

namespace rainy::async {
    struct unset_return_value {
        unset_return_value() {
        }
        unset_return_value(unset_return_value &&) = delete;
        unset_return_value(const unset_return_value &) = delete;
        unset_return_value &operator=(unset_return_value &&) = delete;
        unset_return_value &operator=(const unset_return_value &) = delete;
    };

    class sync_wait_event : type_traits::helper::non_copyable, type_traits::helper::non_moveable {
    public:
        sync_wait_event(bool initially_set = false) : set_{initially_set} {
        }

        ~sync_wait_event() = default;

        rain_fn set() noexcept -> void {
            {
                std::unique_lock<std::mutex> lk{mtx_};
                set_.exchange(true, std::memory_order::seq_cst);
                cond_.notify_all();
            }
        }

        rain_fn reset() noexcept -> void {
            set_.exchange(false, std::memory_order::seq_cst);
        }

        rain_fn wait() noexcept -> void {
            std::unique_lock<std::mutex> lk{mtx_};
            cond_.wait(lk, [this] { return set_.load(std::memory_order::seq_cst); });
        }

    private:
        std::mutex mtx_;
        std::condition_variable cond_;
        std::atomic<bool> set_{false};
    };

    class sync_wait_task_promise_base {
    public:
        sync_wait_task_promise_base() noexcept = default;

        ~sync_wait_task_promise_base() = default;

        static std::suspend_always initial_suspend() noexcept {
            return {};
        }

        auto unhandled_exception() -> void {
            exception = std::current_exception();
        }

    protected:
        sync_wait_event *event{nullptr};
        std::exception_ptr exception;
    };

    template <typename return_type>
    class sync_wait_task_promise : public sync_wait_task_promise_base {
    public:
        using coroutine_type = std::coroutine_handle<sync_wait_task_promise<return_type>>;

        static constexpr bool return_type_is_reference = type_traits::composite_types::is_reference_v<return_type>;
        using stored_type =
            std::conditional_t<return_type_is_reference, type_traits::reference_modify::remove_reference_t<return_type> *,
                               type_traits::cv_modify::remove_const_t<return_type>>;
        using variant_type = std::variant<unset_return_value, stored_type, std::exception_ptr>;

        sync_wait_task_promise() noexcept = default;
        sync_wait_task_promise(const sync_wait_task_promise &) = delete;
        sync_wait_task_promise(sync_wait_task_promise &&) = delete;
        auto operator=(const sync_wait_task_promise &) -> sync_wait_task_promise & = delete;
        auto operator=(sync_wait_task_promise &&) -> sync_wait_task_promise & = delete;
        ~sync_wait_task_promise() = default;

        auto start(sync_wait_event &event) {
            this->event = &event;
            coroutine_type::from_promise(*this).resume();
        }

        auto get_return_object() noexcept {
            return coroutine_type::from_promise(*this);
        }

        template <typename value_type>
            requires(return_type_is_reference && std::is_constructible_v<return_type, value_type &&>) ||
                    (!return_type_is_reference && std::is_constructible_v<stored_type, value_type &&>)
        rain_fn return_value(value_type &&value) -> void {
            if constexpr (return_type_is_reference) {
                return_type ref = static_cast<value_type &&>(value);
                storage.template emplace<stored_type>(utility::addressof(ref));
            } else {
                storage.template emplace<stored_type>(utility::forward<value_type>(value));
            }
        }

        rain_fn return_value(stored_type value) -> void
            requires(!return_type_is_reference)
        {
            if constexpr (std::is_move_constructible_v<stored_type>) {
                storage.template emplace<stored_type>(utility::move(value));
            } else {
                storage.template emplace<stored_type>(value);
            }
        }

        auto final_suspend() const noexcept {
            struct completion_notifier {
                static bool await_ready() noexcept {
                    return false;
                }

                static void await_suspend(coroutine_type coroutine) noexcept {
                    coroutine.promise().event->set();
                }

                static void await_resume() noexcept {};
            };
            return completion_notifier{};
        }

        rain_fn result() -> decltype(auto) {
            if (std::holds_alternative<stored_type>(storage)) {
                if constexpr (return_type_is_reference) {
                    return static_cast<return_type>(*std::get<stored_type>(storage));
                } else {
                    return static_cast<const return_type &>(std::get<stored_type>(storage));
                }
            }
            if (std::holds_alternative<std::exception_ptr>(storage)) {
                std::rethrow_exception(std::get<std::exception_ptr>(storage));
            }
            foundation::exceptions::runtime::throw_runtime_error("The return value was never set, did you execute the coroutine?");
            std::terminate();
        }

        rain_fn result() const -> decltype(auto) {
            if (std::holds_alternative<stored_type>(storage)) {
                if constexpr (return_type_is_reference) {
                    return static_cast<std::add_const_t<return_type>>(*std::get<stored_type>(storage));
                } else {
                    return static_cast<const return_type &>(std::get<stored_type>(storage));
                }
            }
            if (std::holds_alternative<std::exception_ptr>(storage)) {
                std::rethrow_exception(std::get<std::exception_ptr>(storage));
            }
            foundation::exceptions::runtime::throw_runtime_error("The return value was never set, did you execute the coroutine?");
            std::terminate();
        }

    private:
        variant_type storage{};
    };

    template <>
    class sync_wait_task_promise<void> : public sync_wait_task_promise_base {
    public:
        using coroutine_type = std::coroutine_handle<sync_wait_task_promise>;

        sync_wait_task_promise() noexcept = default;
        ~sync_wait_task_promise() = default;

        auto start(sync_wait_event &event) {
            this->event = &event;
            coroutine_type::from_promise(*this).resume();
        }

        auto get_return_object() noexcept {
            return coroutine_type::from_promise(*this);
        }

        static auto final_suspend() noexcept {
            struct completion_notifier {
                static auto await_ready() noexcept {
                    return false;
                }

                static auto await_suspend(const coroutine_type coroutine) noexcept {
                    coroutine.promise().event->set();
                }

                static auto await_resume() noexcept {};
            };

            return completion_notifier{};
        }

        static void return_void() noexcept {
        }

        void result() const {
            if (exception) {
                std::rethrow_exception(exception);
            }
        }
    };

    template <typename return_type>
    class sync_wait_task {
    public:
        using promise_type = sync_wait_task_promise<return_type>;
        using coroutine_type = std::coroutine_handle<promise_type>;

        sync_wait_task(coroutine_type coroutine) noexcept : coroutine(coroutine) {
        }

        sync_wait_task(const sync_wait_task &) = delete;
        sync_wait_task(sync_wait_task &&other) noexcept : coroutine(std::exchange(other.coroutine, coroutine_type{})) {
        }

        sync_wait_task &operator=(const sync_wait_task &) = delete;

        sync_wait_task &operator=(sync_wait_task &&other) noexcept {
            if (std::addressof(other) != this) {
                coroutine = std::exchange(other.coroutine, coroutine_type{});
            }

            return *this;
        }

        ~sync_wait_task() {
            if (coroutine) {
                coroutine.destroy();
            }
        }

        auto &promise() {
            return coroutine.promise();
        }

        const auto &promise() const {
            return coroutine.promise();
        }

    private:
        coroutine_type coroutine;
    };

    template <type_traits::concepts::awaitable awaitable_type,
              typename return_type = typename type_traits::concepts::awaitable_traits<awaitable_type>::awaiter_return_type>
    static auto make_sync_wait_task(awaitable_type &&a) -> sync_wait_task<return_type> {
        if constexpr (type_traits::primary_types::is_void_v<return_type>) {
            co_await utility::forward<awaitable_type>(a);
            co_return;
        } else {
            co_return co_await utility::forward<awaitable_type>(a);
        }
    }
}

namespace rainy::async {
    template <typename return_type>
    class coroutine_task {
    public:
        using task_type = coroutine_task;
        using promise_type = promise<return_type>;
        using coroutine_handle = std::coroutine_handle<promise_type>;

        struct awaitable {
            awaitable(coroutine_handle coroutine) noexcept : coro(coroutine) {
            }

            bool await_ready() const noexcept {
                return !coro || coro.done();
            }

            return_type await_resume() {
                return this->coro.promise().result();
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept {
                coro.promise().continuation(awaiting_coroutine);
                return coro;
            }

            std::coroutine_handle<promise_type> coro{nullptr};
        };

        coroutine_task() noexcept : coro(nullptr) {
        }

        explicit coroutine_task(coroutine_handle handle) : coro(handle) {
        }

        coroutine_task(const coroutine_task &) = delete;

        coroutine_task(coroutine_task &&other) noexcept : coro(std::exchange(other.coro, nullptr)) {
        }

        ~coroutine_task() {
            if (coro != nullptr) {
                coro.destroy();
            }
        }

        coroutine_task &operator=(const coroutine_task &) = delete;

        coroutine_task &operator=(coroutine_task &&other) noexcept {
            if (utility::addressof(other) != this) {
                if (coro != nullptr) {
                    coro.destroy();
                }
                coro = std::exchange(other.coro, nullptr);
            }
            return *this;
        }

        bool is_ready() const noexcept {
            return coro == nullptr || coro.done();
        }

        bool resume() {
            if (!coro.done()) {
                coro.resume();
            }
            return !coro.done();
        }

        bool destroy() {
            if (coro != nullptr) {
                coro.destroy();
                coro = nullptr;
                return true;
            }

            return false;
        }

        auto operator co_await() const noexcept {
            return awaitable{coro};
        }

        promise_type &promise() {
            return coro.promise();
        }

        const promise_type &promise() const {
            return coro.promise();
        }

        coroutine_handle handle() {
            return coro;
        }

    private:
        coroutine_handle coro{nullptr};
    };

    template <typename return_type>
    struct promise final : implements::promise_base {
        using task_type = coroutine_task<return_type>;
        using coroutine_handle = std::coroutine_handle<promise>;
        static constexpr bool return_type_is_reference = type_traits::composite_types::is_reference_v<return_type>;
        using stored_type = type_traits::other_trans::conditional_t<return_type_is_reference, std::remove_reference_t<return_type> *,
                                                                    std::remove_const_t<return_type>>;

        using variant_type = std::variant<unset_return_value, stored_type, std::exception_ptr>;

        promise() noexcept = default;
        promise(const promise &) = delete;
        promise(promise &&other) = delete;
        promise &operator=(const promise &) = delete;
        promise &operator=(promise &&other) = delete;
        ~promise() = default;

        coroutine_task<return_type> get_return_object() noexcept {
            return coroutine_task<return_type>{coroutine_handle::from_promise(*this)};
        }

        template <typename value_type>
            requires(return_type_is_reference && std::is_constructible_v<return_type, value_type &&>) ||
                    (!return_type_is_reference && std::is_constructible_v<stored_type, value_type &&>)
        void return_value(value_type &&value) {
            if constexpr (return_type_is_reference) {
                return_type ref = static_cast<value_type &&>(value);
                storage.template emplace<stored_type>(utility::addressof(ref));
            } else {
                storage.template emplace<stored_type>(utility::forward<value_type>(value));
            }
        }

        void return_value(stored_type value)
            requires(!return_type_is_reference)
        {
            if constexpr (std::is_move_constructible_v<stored_type>) {
                storage.template emplace<stored_type>(utility::move(value));
            } else {
                storage.template emplace<stored_type>(value);
            }
        }

        void unhandled_exception() noexcept {
            new (&storage) variant_type(std::current_exception());
        }

        rain_fn result() -> decltype(auto) {
            if (std::holds_alternative<stored_type>(storage)) {
                if constexpr (return_type_is_reference) {
                    return static_cast<return_type>(*std::get<stored_type>(storage));
                } else {
                    return static_cast<const return_type &>(std::get<stored_type>(storage));
                }
            }
            if (std::holds_alternative<std::exception_ptr>(storage)) {
                std::rethrow_exception(std::get<std::exception_ptr>(storage));
            }
            throw std::runtime_error{"The return value was never set, did you execute the coroutine?"};
        }

        rain_fn result() const -> decltype(auto) {
            if (std::holds_alternative<stored_type>(storage)) {
                if constexpr (return_type_is_reference) {
                    return static_cast<std::add_const_t<return_type>>(*std::get<stored_type>(storage));
                } else {
                    return static_cast<const return_type &>(std::get<stored_type>(storage));
                }
            }
            if (std::holds_alternative<std::exception_ptr>(storage)) {
                std::rethrow_exception(std::get<std::exception_ptr>(storage));
            }
            throw std::runtime_error{"The return value was never set, did you execute the coroutine?"};
        }

    private:
        variant_type storage{};
    };

    template <>
    struct promise<void> : implements::promise_base {
        using task_type = coroutine_task<>;
        using coroutine_handle = std::coroutine_handle<promise>;

        promise() noexcept = default;
        promise(const promise &) = delete;
        promise(promise &&other) = delete;
        promise &operator=(const promise &) = delete;
        promise &operator=(promise &&other) = delete;
        ~promise() = default;

        auto get_return_object() noexcept {
            return coroutine_task<>{coroutine_handle::from_promise(*this)};
        }

        static void return_void() noexcept {
        }

        void unhandled_exception() noexcept {
            exception_ptr = std::current_exception();
        }

        void result() const {
            if (exception_ptr) {
                std::rethrow_exception(exception_ptr);
            }
        }

    private:
        std::exception_ptr exception_ptr{nullptr};
    };

    template <type_traits::concepts::awaitable awaitable_type,
              typename return_type = typename type_traits::concepts::awaitable_traits<awaitable_type>::awaiter_return_type>
    rain_fn sync_wait(awaitable_type &&awaitable) -> decltype(auto) {
        sync_wait_event event{};
        auto task = make_sync_wait_task(utility::forward<awaitable_type>(awaitable));
        task.promise().start(event);
        event.wait();
        if constexpr (type_traits::type_relations::is_void_v<return_type>) {
            task.promise().result();
        } else if constexpr (type_traits::composite_types::is_reference_v<return_type>) {
            return task.promise().result();
        } else if constexpr (type_traits::type_properties::is_move_assignable_v<return_type>) {
            auto result = utility::move(task).promise().result();
            return result;
        } else {
            return task.promise().result();
        }
    }
}

namespace rainy::async {
    template <typename T>
    class generator;

}

namespace rainy::async::implements {
    template <typename T>
    class generator_promise {
    public:
        using value_type = std::remove_reference_t<T>;
        using reference_type = std::conditional_t<std::is_reference_v<T>, T, T &>;
        using pointer_type = value_type *;

        generator_promise() = default;

        auto get_return_object() noexcept -> generator<T>;

        auto initial_suspend() const {
            return std::suspend_always{};
        }

        auto final_suspend() const noexcept(true) {
            return std::suspend_always{};
        }

        template <typename U = T, std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
        auto yield_value(std::remove_reference_t<T> &value) noexcept {
            m_value = std::addressof(value);
            return std::suspend_always{};
        }

        auto yield_value(std::remove_reference_t<T> &&value) noexcept {
            m_value = std::addressof(value);
            return std::suspend_always{};
        }

        auto unhandled_exception() -> void {
            m_exception = std::current_exception();
        }

        auto return_void() noexcept -> void {
        }

        auto value() const noexcept -> reference_type {
            return static_cast<reference_type>(*m_value);
        }

        template <typename U>
        auto await_transform(U &&value) -> std::suspend_never = delete;

        auto rethrow_if_exception() -> void {
            if (m_exception) {
                std::rethrow_exception(m_exception);
            }
        }

    private:
        pointer_type m_value{nullptr};
        std::exception_ptr m_exception;
    };

    struct generator_sentinel {};

    template <typename T>
    class generator_iterator {
        using coroutine_handle = std::coroutine_handle<generator_promise<T>>;

    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = typename generator_promise<T>::value_type;
        using reference = typename generator_promise<T>::reference_type;
        using pointer = typename generator_promise<T>::pointer_type;

        generator_iterator() noexcept {
        }

        explicit generator_iterator(coroutine_handle coroutine) noexcept : m_coroutine(coroutine) {
        }

        friend auto operator==(const generator_iterator &it, generator_sentinel) noexcept -> bool {
            return it.m_coroutine == nullptr || it.m_coroutine.done();
        }

        friend auto operator!=(const generator_iterator &it, generator_sentinel s) noexcept -> bool {
            return !(it == s);
        }

        friend auto operator==(generator_sentinel s, const generator_iterator &it) noexcept -> bool {
            return (it == s);
        }

        friend auto operator!=(generator_sentinel s, const generator_iterator &it) noexcept -> bool {
            return it != s;
        }

        generator_iterator &operator++() {
            m_coroutine.resume();
            if (m_coroutine.done()) {
                m_coroutine.promise().rethrow_if_exception();
            }

            return *this;
        }

        auto operator++(int) -> void {
            (void) operator++();
        }

        reference operator*() const noexcept {
            return m_coroutine.promise().value();
        }

        pointer operator->() const noexcept {
            return std::addressof(operator*());
        }

    private:
        coroutine_handle m_coroutine{nullptr};
    };
}

namespace rainy::async {
    template <typename T>
    class generator : public std::ranges::view_base {
    public:
        using promise_type = implements::generator_promise<T>;
        using iterator = implements::generator_iterator<T>;
        using sentinel = implements::generator_sentinel;
        
        friend class implements::generator_promise<T>;

        generator() noexcept : m_coroutine(nullptr) {
        }

        generator(const generator &) = delete;
        generator(generator &&other) noexcept : m_coroutine(other.m_coroutine) {
            other.m_coroutine = nullptr;
        }

        auto operator=(const generator &) = delete;
        auto operator=(generator &&other) noexcept -> generator & {
            m_coroutine = other.m_coroutine;
            other.m_coroutine = nullptr;

            return *this;
        }

        ~generator() {
            if (m_coroutine) {
                m_coroutine.destroy();
            }
        }

        auto begin() -> iterator {
            if (m_coroutine != nullptr) {
                m_coroutine.resume();
                if (m_coroutine.done()) {
                    m_coroutine.promise().rethrow_if_exception();
                }
            }

            return iterator{m_coroutine};
        }

        auto end() noexcept -> sentinel {
            return sentinel{};
        }

    private:
        explicit generator(std::coroutine_handle<promise_type> coroutine) noexcept : m_coroutine(coroutine) {
        }

        std::coroutine_handle<promise_type> m_coroutine;
    };
}

namespace rainy::async::implements {
    template <typename T>
    auto generator_promise<T>::get_return_object() noexcept -> generator<T> {
        return generator<T>{std::coroutine_handle<generator_promise<T>>::from_promise(*this)};
    }
}

#endif

#endif