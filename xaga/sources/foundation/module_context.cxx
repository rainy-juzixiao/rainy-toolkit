/*
 * Copyright 2025 rainy-juzixiao
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
#include <filesystem>
#include <rainy/foundation/pal/module_context.hpp>

namespace rainy::foundation::pal::module_context {
    struct context::context_private {
        core::handle handle{0};
        bool is_observe{false};
    };

    context::context_private *context::create_ctx() {
        return new context_private; // NOLINT
    }

    void context::destroy_ctx(context_private const *ctx) {
        if (ctx) { // NOLINT
            delete ctx;
        }
    }

    context::context(const std::string_view module_path, const bool load) noexcept {
        namespace fs = std::filesystem;
        rainy_const ctx = create_ctx();
        if (load) {
            ctx->handle = implements::try_to_get_module(module_path, true);
        } else {
            ctx->handle = implements::load_module(module_path, true);
            ctx->is_observe = true;
        }
        if (!ctx->handle) {
            destroy_ctx(ctx);
            return;
        }
        this->private_ = ctx;
    }

    context::context(context &&right) noexcept {
        this->private_ = utility::exchange(right.private_, {});
    }

    context &context::operator=(context &&right) noexcept {
        if (this == &right) {
            return *this;
        }
        release();
        if (this->private_) {
            private_->handle = utility::exchange(right.private_->handle, 0);
            private_->is_observe = private_->is_observe;
            right.private_->is_observe = false;
        } else {
            this->private_ = utility::exchange(right.private_, nullptr);
        }
        return *this;
    }

    context::~context() {
        release();
        if (private_) {
            destroy_ctx(private_);
            private_ = nullptr;
        }
    }

    bool context::release() noexcept { // NOLINT
        if (is_loaded()) {
            if (!is_observe()) {
                if (!implements::release_module(&private_->handle)) {
                    std::terminate(); // 不应当发生的情况
                }
            }
            private_->handle = {};
            private_->is_observe = false;
            return true;
        }
        return false;
    }

    bool context::load(const std::string_view module_path) noexcept {
        namespace fs = std::filesystem;
        release();
        if (!private_) {
            this->private_ = create_ctx();
        }
        private_->handle = implements::load_module(module_path, true);
        if (!private_->handle) {
            return false;
        }
        private_->is_observe = false;
        return true;
    }

    bool context::try_get_module(const std::string_view module_path) noexcept {
        namespace fs = std::filesystem;
        release();
        if (!private_) {
            this->private_ = create_ctx();
        }
        private_->handle = implements::try_to_get_module(module_path, true);
        if (!private_->handle) {
            return false;
        }
        private_->is_observe = true;
        return true;
    }

    farproc_fn context::load_symbol(const std::string_view symbol_name) const noexcept {
        if (is_loaded()) {
            return implements::load_symbol(private_->handle, symbol_name);
        }
        return nullptr;
    }

    bool context::is_loaded() const noexcept {
        if (!private_) {
            return false;
        }
        return private_->handle != 0;
    }

    bool context::is_observe() const noexcept {
        if (is_loaded()) {
            return private_->is_observe;
        }
        return false;
    }

    void *context::native_handle() const noexcept {
        if (is_loaded()) {
            return reinterpret_cast<void *>(private_->handle);
        }
        return nullptr;
    }
}
