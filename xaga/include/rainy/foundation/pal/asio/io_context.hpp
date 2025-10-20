#ifndef RAINY_FOUNDATION_PAL_ASIO_IO_CONTEXT_HPP
#define RAINY_FOUNDATION_PAL_ASIO_IO_CONTEXT_HPP
#include <iostream>
#include <rainy/core/core.hpp>
#if RAINY_USING_WINDOWS
#include <rainy/foundation/pal/asio/timer/queue_set.hpp>
#include <rainy/foundation/pal/threading.hpp>
#include <rainy/foundation/functional/functional.hpp>

namespace rainy::foundation::pal::asio {
    using task_func = void (*)(void *);
    using event_callback = void (*)(core::handle, std::size_t);
}

namespace rainy::foundation::pal::asio::implements {
    struct callback_t {
        using callback_type = rainy::foundation::functional::delegate<void(core::errno_t last_error, std::size_t byte_transfered)>;

        callback_t() {
        }

        template <typename Callback>
        callback_t(Callback &&cb) : cb(std::make_unique<callback_type>(utility::forward<Callback>(cb))) {
        }

        void invoke_callback(DWORD last_error, DWORD byte_transfered);

        std::unique_ptr<callback_type> cb;
    };

    using io_ctx = core::handle;
    RAINY_TOOLKIT_API io_ctx create_io_context();
    RAINY_TOOLKIT_API void destroy_io_context(io_ctx ctx);
    RAINY_TOOLKIT_API int register_io(io_ctx ctx, core::handle publisher_sock, core::handle key);
    RAINY_TOOLKIT_API int unregister_io(io_ctx ctx, core::handle sock);
    RAINY_TOOLKIT_API void stop_io_context(io_ctx ctx);
    RAINY_TOOLKIT_API core::handle extract_handle(io_ctx ctx);
    RAINY_TOOLKIT_API core::handle extract_overlapped(io_ctx ctx);
    RAINY_TOOLKIT_API void post_task(io_ctx ctx, task_func func, void *arg);
    RAINY_TOOLKIT_API int run_io_context(io_ctx ctx);
    RAINY_TOOLKIT_API int run_one_io_context(io_ctx ctx);
    RAINY_TOOLKIT_API int poll_io_context(io_ctx ctx);
    RAINY_TOOLKIT_API int poll_one_io_context(io_ctx ctx);
    RAINY_TOOLKIT_API void reset_io_context(io_ctx ctx);
    RAINY_TOOLKIT_API bool io_context_stopped(io_ctx ctx);
    RAINY_TOOLKIT_API core::handle get_callback(callback_t &&callback);
    using io_context_thread_info = threading::implements::thread_info_base;
}

namespace rainy::foundation::pal::asio {
    class io_context : public threading::thread_context {
    public:
        using event_type = task_func;
        using event_callback = asio::event_callback;
        using handle_type = core::handle;

        io_context() : ctx_(implements::create_io_context()) {
        }

        ~io_context() {
            if (ctx_) {
                implements::destroy_io_context(ctx_);
                ctx_ = 0;
            }
        }

        io_context(const io_context &) = delete;
        io_context &operator=(const io_context &) = delete;

        io_context(io_context &&other) noexcept : ctx_(other.ctx_) {
            other.ctx_ = 0;
        }

        io_context &operator=(io_context &&other) noexcept {
            if (this != &other) {
                if (ctx_) {
                    implements::destroy_io_context(ctx_);
                }
                ctx_ = other.ctx_;
                other.ctx_ = 0;
            }
            return *this;
        }

        void bind_callback(handle_type publisher_sock, std::uintptr_t key = 0) {
            implements::register_io(ctx_, publisher_sock, key);
        }

        void post(task_func func, void *arg) {
            implements::post_task(ctx_, func, arg);
        }

        void dispatch(task_func func, void *arg) {
            if (can_dispatch()) {
                func(arg);
            } else {
                post(func, arg);
            }
        }

        int run() {
            implements::io_context_thread_info this_thread;
            thread_call_stack::context context_(this, this_thread);
            return implements::run_io_context(ctx_);
        }

        int run_one() {
            implements::io_context_thread_info this_thread;
            thread_call_stack::context context_(this, this_thread);
            return implements::run_one_io_context(ctx_);
        }

        int poll() {
            implements::io_context_thread_info this_thread;
            thread_call_stack::context context_(this, this_thread);
            return implements::poll_io_context(ctx_);
        }

        int poll_one() {
            implements::io_context_thread_info this_thread;
            thread_call_stack::context context_(this, this_thread);
            return implements::poll_one_io_context(ctx_);
        }

        void restart() {
            implements::reset_io_context(ctx_);
        }

        void reset() {
            implements::reset_io_context(ctx_);
        }

        void stop() {
            implements::stop_io_context(ctx_);
        }

        handle_type native_handle() const {
            return implements::extract_handle(ctx_);
        }

        handle_type overlapped() const {
            return implements::extract_overlapped(ctx_);
        }

        bool stopped() const noexcept {
            return implements::io_context_stopped(ctx_);
        }

        bool can_dispatch() noexcept {
            return threading::thread_context::thread_call_stack::contains(this) != nullptr;
        }

    private:
        handle_type ctx_{0};
    };
}

#endif

#endif