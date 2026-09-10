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
#include <rainy/foundation/io/ipc/semaphore.hpp>

namespace rainy::foundation::io::ipc::semaphore {
    semaphore::semaphore(executor_type ex, core::memory::nebula_ptr<implements::semaphore_impl_base> impl) noexcept
        : executor_(utility::move(ex)), impl_(utility::move(impl)) {
    }

    semaphore::semaphore(semaphore &&other) noexcept
        : executor_(utility::move(other.executor_)), impl_(utility::move(other.impl_)) {
    }

    semaphore &semaphore::operator=(semaphore &&other) noexcept {
        if (this != &other) {
            executor_ = utility::move(other.executor_);
            impl_ = utility::move(other.impl_);
        }
        return *this;
    }

    semaphore::~semaphore() = default;

    semaphore semaphore::create(executor_type ex, core::text::string_view name, const attributes &attr) {
        std::error_code ec;
        auto result = create(ex, name, attr, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    semaphore semaphore::create(executor_type ex, core::text::string_view name, const attributes &attr,
                                 std::error_code &ec) {
        return semaphore(ex, implements::create_semaphore_impl(ex, name.data(), attr, ec));
    }

    semaphore semaphore::open(executor_type ex, core::text::string_view name) {
        std::error_code ec;
        auto result = open(ex, name, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    semaphore semaphore::open(executor_type ex, core::text::string_view name, std::error_code &ec) {
        return semaphore(ex, implements::open_semaphore_impl(ex, name.data(), ec));
    }

    semaphore semaphore::open_or_create(executor_type ex, core::text::string_view name, const attributes &attr) {
        std::error_code ec;
        auto result = open_or_create(ex, name, attr, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    semaphore semaphore::open_or_create(executor_type ex, core::text::string_view name, const attributes &attr,
                                         std::error_code &ec) {
        return semaphore(ex, implements::open_or_create_semaphore_impl(ex, name.data(), attr, ec));
    }

    std::error_code semaphore::unlink(core::text::string_view name) {
        return implements::unlink_semaphore(name.data());
    }

    std::error_code semaphore::wait() noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->wait();
    }

    std::error_code semaphore::try_wait() noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->try_wait();
    }

    std::error_code semaphore::timed_wait(std::chrono::milliseconds timeout) noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->timed_wait(timeout);
    }

    std::error_code semaphore::post() noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->post();
    }

    bool semaphore::is_open() const noexcept {
        return impl_ && impl_->is_open();
    }

    std::error_code semaphore::close() noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->close();
    }

    semaphore::native_handle_type semaphore::native_handle() const noexcept {
        if (!impl_) {
#if RAINY_USING_WINDOWS
            return reinterpret_cast<native_handle_type>(-1);
#else
            return static_cast<native_handle_type>(-1);
#endif
        }
        return impl_->native_handle();
    }

    semaphore::executor_type semaphore::get_executor() const noexcept {
        return executor_;
    }

    void semaphore::swap(semaphore &other) noexcept {
        using std::swap;
        swap(executor_, other.executor_);
        swap(impl_, other.impl_);
    }

    void swap(semaphore &a, semaphore &b) noexcept {
        a.swap(b);
    }
}