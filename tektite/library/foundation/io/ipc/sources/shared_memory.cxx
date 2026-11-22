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
#include <rainy/foundation/io/ipc/shared_memory.hpp>

namespace rainy::foundation::io::ipc::shared_memory {
    shared_memory::shared_memory(executor_type ex, core::memory::nebula_ptr<implements::shared_memory_impl_base> impl) noexcept :
        executor_(utility::move(ex)), impl_(utility::move(impl)) {
    }

    shared_memory::shared_memory(shared_memory &&other) noexcept :
        executor_(utility::move(other.executor_)), impl_(utility::move(other.impl_)) {
    }

    shared_memory &shared_memory::operator=(shared_memory &&other) noexcept {
        if (this != &other) {
            executor_ = utility::move(other.executor_);
            impl_ = utility::move(other.impl_);
        }
        return *this;
    }

    shared_memory::~shared_memory() = default;

    shared_memory shared_memory::create(executor_type ex, core::text::string_view name, const attributes &attr) {
        std::error_code ec;
        auto result = create(ex, name, attr, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    shared_memory shared_memory::create(executor_type ex, core::text::string_view name, const attributes &attr, std::error_code &ec) {
        return shared_memory(ex, implements::create_shared_memory_impl(ex, name.data(), attr, ec));
    }

    shared_memory shared_memory::open(executor_type ex, core::text::string_view name, access_mode mode) {
        std::error_code ec;
        auto result = open(ex, name, mode, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    shared_memory shared_memory::open(executor_type ex, core::text::string_view name, access_mode mode, std::error_code &ec) {
        return shared_memory(ex, implements::open_shared_memory_impl(ex, name.data(), mode, ec));
    }

    shared_memory shared_memory::open_or_create(executor_type ex, core::text::string_view name, const attributes &attr) {
        std::error_code ec;
        auto result = open_or_create(ex, name, attr, ec);
        if (ec) {
            core::exceptions::runtime::throw_system_error(ec);
        }
        return result;
    }

    shared_memory shared_memory::open_or_create(executor_type ex, core::text::string_view name, const attributes &attr,
                                                std::error_code &ec) {
        return shared_memory(ex, implements::open_or_create_shared_memory_impl(ex, name.data(), attr, ec));
    }

    std::error_code shared_memory::unlink(core::text::string_view name) {
        return implements::unlink_shared_memory(name.data());
    }

    shared_memory::pointer shared_memory::data() const noexcept {
        if (!impl_) {
            return nullptr;
        }
        return static_cast<pointer>(impl_->data());
    }

    shared_memory::size_type shared_memory::size() const noexcept {
        if (!impl_) {
            return 0;
        }
        return impl_->size();
    }

    bool shared_memory::empty() const noexcept {
        return size() == 0;
    }

    std::error_code shared_memory::flush() noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->flush();
    }

    std::error_code shared_memory::flush(size_type offset, size_type length) noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->flush(offset, length);
    }

    bool shared_memory::is_open() const noexcept {
        return impl_ && impl_->is_open();
    }

    std::error_code shared_memory::close() noexcept {
        if (!impl_) {
            return std::make_error_code(std::errc::bad_file_descriptor);
        }
        return impl_->close();
    }

    shared_memory::native_handle_type shared_memory::native_handle() const noexcept {
        if (!impl_) {
#if RAINY_USING_WINDOWS
            return reinterpret_cast<native_handle_type>(-1);
#else
            return static_cast<native_handle_type>(-1);
#endif
        }
        return impl_->native_handle();
    }

    shared_memory::executor_type shared_memory::get_executor() const noexcept {
        return executor_;
    }

    void shared_memory::swap(shared_memory &other) noexcept {
        using std::swap;
        swap(executor_, other.executor_);
        swap(impl_, other.impl_);
    }

    void swap(shared_memory &a, shared_memory &b) noexcept {
        a.swap(b);
    }
}
