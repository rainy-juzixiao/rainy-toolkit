#include <rainy/foundation/concurrency/mutex.hpp>

namespace rainy::foundation::concurrency {
    mutex::~mutex() {
        if (mtx_) {
            implements::mtx_destroy(&mtx_);
            mtx_ = nullptr;
        }
    }

    rain_fn mutex::lock() -> void {
        if (rainy_const r = implements::mtx_lock(&mtx_); r != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn mutex::try_lock() noexcept -> bool {
        rainy_const r = implements::mtx_trylock(&mtx_);

        if (r == thrd_result::success) {
            return true;
        }
        if (r == thrd_result::busy) {
            return false;
        }

        std::terminate();
    }

    rain_fn mutex::unlock() -> void { // NOLINT
        if (rainy_const r = implements::mtx_unlock(&mtx_); r != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn mutex::native_handle() noexcept -> native_handle_type { // NOLINT
        return implements::native_mtx_handle(&mtx_);
    }
}

namespace rainy::foundation::concurrency {
    recursive_mutex::recursive_mutex() noexcept {
        constexpr int flags =
            implements::mutex_types::plain_mtx | implements::mutex_types::try_mtx | implements::mutex_types::recursive_mtx;

        if (const auto r = implements::mtx_create(&mtx_, flags); r != thrd_result::success) {
            std::terminate();
        }
    }

    recursive_mutex::~recursive_mutex() {
        if (!mtx_) {
            return;
        }

        if (const auto r = implements::mtx_destroy(&mtx_); r != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn recursive_mutex::lock() -> void {
        if (const auto r = implements::mtx_lock(&mtx_); r != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn recursive_mutex::try_lock() noexcept -> bool {
        switch (const auto r = implements::mtx_trylock(&mtx_); r) {
            case thrd_result::success:
                return true;
            case thrd_result::busy:
                return false;
            default:
                std::terminate();
        }
    }

    rain_fn recursive_mutex::unlock() -> void { // NOLINT
        if (const auto r = implements::mtx_unlock(&mtx_); r != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn recursive_mutex::native_handle() noexcept -> native_handle_type {
        return &mtx_;
    }
}

namespace rainy::foundation::concurrency {
    timed_mutex::timed_mutex() noexcept {
        constexpr int flags =
            implements::mutex_types::plain_mtx | implements::mutex_types::try_mtx | implements::mutex_types::timed_mtx;

        if (implements::mtx_create(&mtx_, flags) != thrd_result::success) {
            std::terminate();
        }
    }

    timed_mutex::~timed_mutex() {
        if (!mtx_) {
            return;
        }

        if (implements::mtx_destroy(&mtx_) != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn timed_mutex::lock() -> void {
        if (implements::mtx_lock(&mtx_) != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn timed_mutex::try_lock() -> bool {
        const auto r = implements::mtx_trylock(&mtx_);

        if (r == thrd_result::success) {
            return true;
        }
        if (r == thrd_result::busy) {
            return false;
        }

        std::terminate();
    }

    rain_fn timed_mutex::unlock() -> void { // NOLINT
        if (implements::mtx_unlock(&mtx_) != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn timed_mutex::native_handle() noexcept -> native_handle_type {
        return &mtx_;
    }
}
namespace rainy::foundation::concurrency {
    recursive_timed_mutex::recursive_timed_mutex() noexcept {
        constexpr int flags = implements::mutex_types::plain_mtx | implements::mutex_types::try_mtx |
                              implements::mutex_types::timed_mtx | implements::mutex_types::recursive_mtx;

        if (implements::mtx_create(&mtx_, flags) != thrd_result::success) {
            std::terminate();
        }
    }

    recursive_timed_mutex::~recursive_timed_mutex() {
        if (!mtx_) {
            return;
        }

        if (implements::mtx_destroy(&mtx_) != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn recursive_timed_mutex::lock() -> void {
        if (implements::mtx_lock(&mtx_) != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn recursive_timed_mutex::try_lock() noexcept -> bool {
        const auto r = implements::mtx_trylock(&mtx_);
        if (r == thrd_result::success) {
            return true;
        }
        if (r == thrd_result::busy) {
            return false;
        }
        std::terminate();
    }

    rain_fn recursive_timed_mutex::unlock() -> void { // NOLINT
        if (implements::mtx_unlock(&mtx_) != thrd_result::success) {
            std::terminate();
        }
    }

    rain_fn recursive_timed_mutex::native_handle() noexcept -> native_handle_type {
        return &mtx_;
    }

}
