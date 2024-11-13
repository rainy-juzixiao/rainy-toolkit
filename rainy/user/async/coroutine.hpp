#ifndef RAINY_COROUTINE_H
#define RAINY_COROUTINE_H

#include <rainy/core.hpp>

#if RAINY_HAS_CXX20
#include <condition_variable>
#include <coroutine>
#include <rainy/system/exceptions.hpp>
#include <rainy/user/async/coro_type_traits.hpp>
#include <stdexcept>
#include <utility>
#include <variant>

namespace rainy::user::async {
    template <typename return_type = void>
    class coroutine_task;
}

namespace rainy::user::async::utils {
    struct unset_return_value {
        unset_return_value() {
        }
        unset_return_value(unset_return_value &&) = delete;
        unset_return_value(const unset_return_value &) = delete;
        unset_return_value &operator=(unset_return_value &&) = delete;
        unset_return_value &operator=(const unset_return_value &) = delete;
    };

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


    class sync_wait_event {
    public:
        sync_wait_event(bool initially_set = false);
        sync_wait_event(const sync_wait_event &) = delete;
        sync_wait_event(sync_wait_event &&) = delete;
        auto operator=(const sync_wait_event &) -> sync_wait_event & = delete;
        auto operator=(sync_wait_event &&) -> sync_wait_event & = delete;
        ~sync_wait_event() = default;

        auto set() noexcept -> void;
        auto reset() noexcept -> void;
        auto wait() noexcept -> void;

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::atomic<bool> m_set{false};
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

        static constexpr bool return_type_is_reference = std::is_reference_v<return_type>;
        using stored_type =
            std::conditional_t<return_type_is_reference, std::remove_reference_t<return_type> *, std::remove_const_t<return_type>>;
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
                    (not return_type_is_reference && std::is_constructible_v<stored_type, value_type &&>)
        auto return_value(value_type &&value) -> void {
            if constexpr (return_type_is_reference) {
                return_type ref = static_cast<value_type &&>(value);
                storage.template emplace<stored_type>(foundation::utility::addressof(ref));
            } else {
                storage.template emplace<stored_type>(foundation::utility::forward<value_type>(value));
            }
        }

        auto return_value(stored_type value) -> void
            requires(!return_type_is_reference)
        {
            if constexpr (std::is_move_constructible_v<stored_type>) {
                storage.template emplace<stored_type>(foundation::utility::move(value));
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

        auto &result() {
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

        const auto &result() const {
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


        auto &&result() {
            if (std::holds_alternative<stored_type>(storage)) {
                if constexpr (return_type_is_reference) {
                    return static_cast<return_type>(*std::get<stored_type>(storage));
                } else if constexpr (std::is_assignable_v<return_type, stored_type>) {
                    return static_cast<return_type &&>(std::get<stored_type>(storage));
                } else {
                    return static_cast<const return_type &&>(std::get<stored_type>(storage));
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
            co_await foundation::utility::forward<awaitable_type>(a);
            co_return;
        } else {
            co_return co_await foundation::utility::forward<awaitable_type>(a);
        }
    }
}

namespace rainy::user::async {
    template <typename return_type>
    struct promise final : utils::promise_base {
        using task_type = coroutine_task<return_type>;
        using coroutine_handle = std::coroutine_handle<promise>;
        static constexpr bool return_type_is_reference = std::is_reference_v<return_type>;
        using stored_type =
            std::conditional_t<return_type_is_reference, std::remove_reference_t<return_type> *, std::remove_const_t<return_type>>;

        using variant_type = std::variant<utils::unset_return_value, stored_type, std::exception_ptr>;

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
                storage.template emplace<stored_type>(foundation::utility::addressof(ref));
            } else {
                storage.template emplace<stored_type>(foundation::utility::forward<value_type>(value));
            }
        }

        void return_value(stored_type value)
            requires(!return_type_is_reference)
        {
            if constexpr (std::is_move_constructible_v<stored_type>) {
                storage.template emplace<stored_type>(foundation::utility::move(value));
            } else {
                storage.template emplace<stored_type>(value);
            }
        }

        void unhandled_exception() noexcept {
            new (&storage) variant_type(std::current_exception());
        }

        auto &result() {
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

        const auto &result() const {
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

        auto &&result() {
            if (std::holds_alternative<stored_type>(storage)) {
                if constexpr (return_type_is_reference) {
                    return static_cast<return_type>(*std::get<stored_type>(storage));
                } else if constexpr (std::is_move_constructible_v<return_type>) {
                    return static_cast<return_type &&>(std::get<stored_type>(storage));
                } else {
                    return static_cast<const return_type &&>(std::get<stored_type>(storage));
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
    struct promise<void> : utils::promise_base {
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
            if (foundation::utility::addressof(other) != this) {
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

    template <type_traits::concepts::awaitable awaitable_type,
              typename return_type = typename type_traits::concepts::awaitable_traits<awaitable_type>::awaiter_return_type>
    auto sync_wait(awaitable_type &&awaitable) {
        utils::sync_wait_event event{};
        auto task = utils::make_sync_wait_task(foundation::utility::forward<awaitable_type>(awaitable));
        task.promise().start(event);
        event.wait();

        if constexpr (std::is_void_v<return_type>) {
            task.promise().result();
        } else if constexpr (std::is_reference_v<return_type>) {
            return task.promise().result();
        } else if constexpr (std::is_move_assignable_v<return_type>) {
            auto result = foundation::utility::move(task).promise().result();
            return result;
        } else {
            return task.promise().result();
        }
    }
}

#endif

#endif
