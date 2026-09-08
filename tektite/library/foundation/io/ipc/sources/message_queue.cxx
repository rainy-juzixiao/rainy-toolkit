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
#include <rainy/foundation/io/ipc/message_queue.hpp>

namespace rainy::foundation::io::ipc::message_queue {
    message_queue::message_queue(message_queue &&other) noexcept :
        executor_(utility::move(other.executor_)), impl_(utility::move(other.impl_)) {
    }

    message_queue &message_queue::operator=(message_queue &&other) noexcept {
        if (this != &other) {
            if (impl_ && impl_->is_open()) {
                utility::ignore = impl_->close();
            }
            executor_ = utility::move(other.executor_);
            impl_ = utility::move(other.impl_);
        }
        return *this;
    }

    message_queue::~message_queue() {
        if (impl_ && impl_->is_open()) {
            utility::ignore = impl_->close();
        }
    }

    message_queue message_queue::create(executor_type ex, core::text::string_view name, const attributes &attr, direction dir) {
        std::error_code ec;
        auto result = create(ex, name, attr, dir, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    message_queue message_queue::create(executor_type ex, core::text::string_view name, const attributes &attr, direction dir,
                                        std::error_code &ec) {
        auto impl = implements::create_message_queue_impl(ex, name.data(), attr, dir, ec);
        return message_queue{ex, utility::move(impl)};
    }

    message_queue message_queue::open(executor_type ex, core::text::string_view name, direction dir) {
        std::error_code ec;
        auto result = open(ex, name, dir, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    message_queue message_queue::open(executor_type ex, core::text::string_view name, direction dir, std::error_code &ec) {
        auto impl = implements::open_message_queue_impl(ex, name.data(), dir, ec);
        return message_queue{ex, utility::move(impl)};
    }

    message_queue message_queue::open_or_create(executor_type ex, core::text::string_view name, const attributes &attr,
                                                 direction dir) {
        std::error_code ec;
        auto result = open_or_create(ex, name, attr, dir, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    message_queue message_queue::open_or_create(executor_type ex, core::text::string_view name, const attributes &attr,
                                                 direction dir, std::error_code &ec) {
        auto impl = implements::open_or_create_message_queue_impl(ex, name.data(), attr, dir, ec);
        return message_queue{ex, utility::move(impl)};
    }

    std::error_code message_queue::unlink(core::text::string_view name) {
        return implements::unlink_message_queue(name.data());
    }

    bool message_queue::empty() const noexcept {
        return impl_ ? impl_->current_messages() == 0 : true;
    }

    message_queue::size_type message_queue::size() const noexcept {
        return impl_ ? impl_->current_messages() : 0;
    }

    message_queue::size_type message_queue::capacity() const noexcept {
        return impl_ ? impl_->max_messages() : 0;
    }

    message_queue::size_type message_queue::max_message_size() const noexcept {
        return impl_ ? impl_->max_message_size() : 0;
    }

    void message_queue::push(const const_buffer &buffer) {
        push(buffer, priority::low);
    }

    void message_queue::push(const const_buffer &buffer, priority priority) {
        std::error_code ec;
        push(buffer, priority, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
    }

    void message_queue::push(const const_buffer &buffer, std::error_code &ec) {
        push(buffer, priority::low, ec);
    }

    void message_queue::push(const const_buffer &buffer, priority priority, std::error_code &ec) {
        if (!impl_) {
            ec = std::make_error_code(std::errc::bad_file_descriptor);
            return;
        }
        auto r = impl_->send(buffer.data(), buffer.size(), priority, ec);
        if (r < 0) {
            return;
        }
        ec.clear();
    }

    bool message_queue::try_push(const const_buffer &buffer, priority priority, std::error_code &ec) {
        if (!impl_) {
            ec = std::make_error_code(std::errc::bad_file_descriptor);
            return false;
        }
        auto r = impl_->try_send(buffer.data(), buffer.size(), priority, ec);
        if (ec) {
            return false;
        }
        return r >= 0;
    }

    bool message_queue::timed_push_ms(const const_buffer &buffer, priority priority, std::chrono::milliseconds timeout,
                                       std::error_code &ec) {
        if (!impl_) {
            ec = std::make_error_code(std::errc::bad_file_descriptor);
            return false;
        }
        auto r = impl_->timed_send(buffer.data(), buffer.size(), priority, timeout, ec);
        if (ec) {
            return false;
        }
        return r >= 0;
    }

    message_queue::size_type message_queue::pop(const mutable_buffer &buffer) {
        priority priority{priority::low};
        return pop(buffer, priority);
    }

    message_queue::size_type message_queue::pop(const mutable_buffer &buffer, priority &priority) {
        std::error_code ec;
        auto n = pop(buffer, priority, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return n;
    }

    message_queue::size_type message_queue::pop(const mutable_buffer &buffer, priority &priority, std::error_code &ec) {
        if (!impl_) {
            ec = std::make_error_code(std::errc::bad_file_descriptor);
            return 0;
        }
        auto r = impl_->receive(buffer.data(), buffer.size(), priority, ec);
        if (r < 0) {
            return 0;
        }
        return static_cast<size_type>(r);
    }

    bool message_queue::try_pop(const mutable_buffer &buffer, priority &priority, size_type &received, std::error_code &ec) {
        if (!impl_) {
            ec = std::make_error_code(std::errc::bad_file_descriptor);
            return false;
        }
        auto r = impl_->try_receive(buffer.data(), buffer.size(), priority, ec);
        if (ec) {
            return false;
        }
        if (r < 0) {
            return false;
        }
        received = static_cast<size_type>(r);
        return true;
    }

    bool message_queue::timed_pop_ms(const mutable_buffer &buffer, priority &priority, size_type &received,
                                      std::chrono::milliseconds timeout, std::error_code &ec) {
        if (!impl_) {
            ec = std::make_error_code(std::errc::bad_file_descriptor);
            return false;
        }
        auto r = impl_->timed_receive(buffer.data(), buffer.size(), priority, timeout, ec);
        if (ec) {
            return false;
        }
        if (r < 0) {
            return false;
        }
        received = static_cast<size_type>(r);
        return true;
    }

    bool message_queue::is_open() const noexcept {
        return impl_ && impl_->is_open();
    }

    std::error_code message_queue::close() noexcept {
        if (!impl_) {
            return {};
        }
        return impl_->close();
    }

    std::error_code message_queue::cancel() noexcept {
        if (!impl_) {
            return {};
        }
        return impl_->cancel();
    }

    message_queue::native_handle_type message_queue::native_handle() const noexcept {
#if RAINY_USING_WINDOWS
        return impl_ ? impl_->native_handle() : reinterpret_cast<native_handle_type>(-1);
#else
        return impl_ ? impl_->native_handle() : static_cast<native_handle_type>(-1);
#endif
    }

    message_queue::executor_type message_queue::get_executor() const noexcept {
        return executor_;
    }

    void message_queue::swap(message_queue &other) noexcept {
        using std::swap;
        swap(executor_, other.executor_);
        swap(impl_, other.impl_);
    }

    message_queue::message_queue(executor_type ex, core::memory::nebula_ptr<implements::message_queue_impl_base> impl) noexcept :
        executor_(ex), impl_(utility::move(impl)) {
    }

    void swap(message_queue &a, message_queue &b) noexcept {
        a.swap(b);
    }
}